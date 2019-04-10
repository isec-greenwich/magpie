import pandas as pd
# import numpy as np
import csv
import multiprocessing
# import socket
import os
import time
import zmq
import json
from datetime import datetime, timedelta
import sys
import math
import magpie_train as reasoning
import magpie_datapreproc as datapreproc
from termcolor import cprint
import subprocess
import warnings

if not sys.warnoptions:
    warnings.simplefilter("ignore")


class Monitor:

    def output(self, header, feed_id, feed_window, filename):
        '''
        A function to output collected MDS data from the universal parser.
        This will eventually be a SQLITE DB, and not output to CSV files.
        '''
        for i, line in enumerate(feed_window):
            if feed_id == "amds":
                feed_window[i] = feed_window[i]
            else:
                feed_window[i] = feed_window[i][:-2]

        file_exists = os.path.isfile("/home/pi/magpie/datastore/" + filename + "." + feed_id + ".csv")
        outfile = open("/home/pi/magpie/datastore/" + filename + "." + feed_id + ".csv", "a")
        writer = csv.writer(outfile)
        if not file_exists:
            writer.writerow(header)
        if feed_id == "amds":
            writer.writerow(feed_window)
        else:
            writer.writerows(feed_window)

    def window_sigmoid(self, pred, sensitivity):
        attack_flags = 0
        normal_flags = 0
        for i in pred:
            if i == -1:
                attack_flags += 1
            else:
                normal_flags += 1
        x = attack_flags / (attack_flags + normal_flags)
        if x == 0.0:
            sigmoid = [x]
        else:
            sigmoid = [x / (sensitivity + x)]
        return sigmoid

    def window_process(self, m_list, i, data, model, parameters):
        cutoff = float(parameters[5][1])
        if i == 'ip' or i == 'wifi' or i == 'zigbee':
            sensitivity = float(parameters[0][1])
        elif i == 'wifi':
            sensitivity = float(parameters[1][1])
        elif i == 'zigbee':
            sensitivity = float(parameters[2][1])
        else:
            sensitivity = cutoff  # default sensitivity for feeds with single sample output per window

        preproc = datapreproc.feature_engineering()
        model_data = preproc.data_preproc_mon(i, data)
        pred = model.predict(model_data)
        sig = self.window_sigmoid(pred, sensitivity)
        temp = sig + [i]
        m_list.append(temp)
        return m_list

    def monitor_state(self, h_df, cutoff, rpp, start_time, mdt, timestamp, activity):
        h_df = h_df.sort_values('feed_id')
        # print(h_df)
        hlist = []

        for x in range(len(h_df.index)):
            hlist.append(h_df['prediction'].iloc[x])

        h1 = hlist[0]
        h2 = hlist[1]
        h3 = hlist[2]
        h4 = hlist[3]
        h5 = hlist[4]
        h1c = h1 ** 3
        h2c = h2 ** 3
        h3c = h3 ** 3
        h4c = h4 ** 3
        h5c = h5 ** 3
        sqr = math.sqrt((h1c + h2c + h3c + h4c + h5c) / (h1 + h2 + h3 + h4 + h5))
        if math.isnan(sqr):
            sqr = 0
        else:
            sqr = round(sqr, 2)
        if sqr > float(cutoff):
            mon_state = 'Anomaly'
            confidence = sqr
        else:
            mon_state = 'Normal'
            confidence = 1-sqr
        if activity == 0:
            activity_state = 'No Activity detected'
        elif activity == 1:
            activity_state = 'Activity detected'
        else:
            activity_state = 'Disabled'
        source_detect = max(hlist)
        if source_detect == 0.0:
            source_id = 'None'
        else:
            source_id = h_df.loc[h_df['prediction'].idxmax(), 'feed_id']
        time_elapsed = datetime.now() - start_time

        # if time_elapsed < mdt:
        #    exceeded = 'No'
        #else:
        #    exceeded = 'Yes'
        window = datetime.utcfromtimestamp(float(timestamp[0])).strftime('%H:%M:%S')
        print('')
        print('-------------------------------------------------------')
        print('| MAGPIE Monitoring Report: ' + str(window) + ' (' + str(timestamp[0]) + ')')
        print('-------------------------------------------------------')
        print('| Risk Profile Policy: ' + rpp)
        print('| Activity Recognition: ' + activity_state)
        if mon_state == 'Anomaly':
            cprint('| Threat State: ' + mon_state + '  - Confidence ('
                   + str(round(confidence, 2)) + ')', 'red', attrs=['bold'])
        else:
            cprint('| Threat State: ' + mon_state + '  - Confidence ('
                   + str(round(confidence, 2)) + ')', 'green', attrs=['bold'])
        print('| Source Detection: ' + str(source_id)
              + '- Confidence (' + str(round(source_detect, 2)) + ')')
        print('| Reasoning Latency: (ss.ms) {}'.format(time_elapsed))
        # print('| MDT Exceeded: ' + exceeded)
        # if exceeded == 'Yes':
        #    select_model = 'low'
        #    cprint('| *** Adjusting reasoning to recover MDT *** '
        #           + exceeded, 'yellow', attrs=['bold'])
        #else:
        #    select_model = 'high'
        print('-------------------------------------------------------')
        file_exists = os.path.isfile("/home/pi/magpie/datastore/monitor_record.csv")
        header = ['timestamp', 'threat_state', 'state_confidence',
                  'mds_source', 'source_confidence', 'reasoning_lat']
        monitor_record = [timestamp[0], mon_state, confidence, source_id, source_detect, time_elapsed]
        outfile = open("/home/pi/magpie/datastore/monitor_record.csv", "a")
        writer = csv.writer(outfile)
        if not file_exists:
            writer.writerow(header)
        else:
            writer.writerow(monitor_record)

    def run_feeds(self, parameters, ip_feed, rf_feed, wifi_feed, zb_feed, audio_feed,
                                     ip_mod, wifi_mod, zb_mod, rf_mod, audio_mod):

        manager = multiprocessing.Manager()
        m_list = manager.list()
        worker_pool = []

        for i in parameters[0:5]:  # no aMDS here, aMDS is for activity recognition only
            if i[0] == 'ip':
                data = ip_feed
                model = ip_mod
            elif i[0] == 'wifi':
                data = wifi_feed
                model = wifi_mod
            elif i[0] == 'zigbee':
                data = zb_feed
                model = zb_mod
            elif i[0] == 'rf':
                data = rf_feed
                model = rf_mod
            elif i[0] == 'audio':
                data = audio_feed
                model = audio_mod
            else:
                print('Error unknown feed - aborting')
                break
            p = multiprocessing.Process(target=self.window_process,
                                        args=(m_list, i[0], data, model, parameters))
            worker_pool.append(p)
            p.start()
        for p in worker_pool:
            p.join()  # Wait for all of the workers to finish.
        return m_list

    def run(self, rpp, mdt, feed_id, activity_check, collect_bin, filename, mode, activity_presence):
        # zigbee header feature column names
        zb_h = ["timestamp", "src", "dest", "type", "freq", "avg_sz", "std_sz", "avg_del", "std_del"]

        # ip header feature column names
        ip_h = ["timestamp", "src", "dest", "src_port", "dest_port", "freq", "avg_ttl",
                "avg_sz", "std_sz", "avg_del", "std_del"]

        # wifi header feature column names
        # removed redundant feature type
        wifi_h = ["timestamp", "src", "dest", "freq", "subtype", "avg_sz", "avg_rssi", "std_rssi",
                  "avg_del", "std_del"]

        # audio header feature column names
        audio_h = ["timestamp", "freq", "avg_rms", "std_rms"]

        # rf spectrum (2.4Ghz) header feature column names
        rf_h = ["timestamp", "avg_bin1",
                "std_bin1", "avg_bin2", "std_bin2", "avg_bin3", "std_bin3", "avg_bin4", "std_bin4", "avg_bin5",
                "std_bin5", "avg_bin6", "std_bin6", "avg_bin7", "std_bin7", "avg_bin8", "std_bin8", "avg_bin9",
                "std_bin9", "avg_bin10", "std_bin10", "avg_bin11", "std_bin11", "avg_bin12", "std_bin12",
                "avg_bin13", "std_bin13", "avg_bin14", "std_bin14", "avg_bin15", "std_bin15", "avg_bin16",
                "std_bin16", "avg_bin17", "std_bin17", "avg_bin18", "std_bin18", "avg_bin19", "std_bin19",
                "avg_bin20", "std_bin20", "avg_bin21", "std_bin21", "avg_bin22", "std_bin22", "avg_bin23",
                "std_bin23", "avg_bin24", "std_bin24", "avg_bin25", "std_bin25", "avg_bin26", "std_bin26",
                "avg_bin27", "std_bin27", "avg_bin28", "std_bin28", "avg_bin29", "std_bin29", "avg_bin30",
                "std_bin30", "avg_bin31", "std_bin31", "avg_bin32", "std_bin32", "avg_bin33", "std_bin33",
                "avg_bin34", "std_bin34", "avg_bin35", "std_bin35", "avg_bin36", "std_bin36", "avg_bin37",
                "std_bin37", "avg_bin38", "std_bin38", "avg_bin39", "std_bin39", "avg_bin40", "std_bin40",
                "avg_bin41", "std_bin41", "avg_bin42", "std_bin42", "avg_bin43", "std_bin43", "avg_bin44",
                "std_bin44", "avg_bin45", "std_bin45", "avg_bin46", "std_bin46", "avg_bin47", "std_bin47"]

        # amds header feature column names
        amds_h = ["ip_sum_freq", "ip_sum_sz", "ip_destp_mode", "ip_destp_least", "ip_avg_freq",
                  "ip_avg_sz", "ip_std_sz", "ip_avg_ttl", "ip_avg_delay", "ip_std_delay", "ip_std_freq",
                  "wifi_sum_freq", "wifi_sum_sz", "wifi_subtype_mode", "wifi_type_least", "wifi_avg_freq",
                  "wifi_avg_sz", "wifi_avg_rssi", "wifi_std_rssi", "wifi_avg_delay", "wifi_std_delay", "wifi_std_freq",
                  "zb_sum_freq", "zb_sum_sz", "zb_type_mode", "zb_type_least", "zb_avg_freq", "zb_avg_sz",
                  "zb_std_sz", "zb_avg_delay", "zb_std_delay", "zb_std_freq",
                  "audio_freq", "audio_avg_rms", "audio_std_rms", "avg_db", "std_db", 'activity']

        # check runtime mode
        run = reasoning.Model()
        reasoning_mod0 = os.path.isfile("/home/pi/magpie/models/0.reasoning.rf.sav")
        reasoning_mod1 = os.path.isfile("/home/pi/magpie/models/1.reasoning.rf.sav")
        reasoning_mod = os.path.isfile("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.rf.sav")
        train_data = os.path.isfile("/home/pi/magpie/datastore/" + str(activity_presence) + ".train.rf.csv")

        ''' This section of code is is extremely bad hack with copy and paste of code, it needs to be changed to
            a set of shared functions, but now for the sake of getting things working ...
        '''
        if mode == 1:
            if reasoning_mod0 or reasoning_mod1:
                if activity_check == 1:
                        if reasoning_mod0 and reasoning_mod1:
                                load = 1
                                cprint("[WARNING] Activity presence recognition: Enabled", 'magenta')
                                ip_mod1, wifi_mod1, zb_mod1, audio_mod1, rf_mod1, \
                                ip_mod2, wifi_mod2, zb_mod2, audio_mod2, rf_mod2, \
                                amds_mod = run.load_models(load, activity_presence)
                                ready = 1
                        else:
                                if reasoning_mod:
                                    load = 2
                                    cprint("[WARNING] Activity presence recognition "
                                           "cannot run without both 'activity' & 'no activity' "
                                           "MDS models", 'yellow')
                                    cprint("[WARNING] Activity presence recognition: Disabled",
                                           'yellow')
                                    ip_mod, wifi_mod, zb_mod, audio_mod, \
                                    rf_mod = run.load_models(load, activity_presence)
                                    ready = 1
                                    activity_check = 0
                                else:
                                    print('[DEBUG] reasoning_mod: ' + str(reasoning_mod))
                                    cprint("[LOADING] MAGPIE Reasoning Engine is not configured", 'yellow')
                                    if train_data:
                                        run.train(feed_id, activity_presence)
                                        load = 2
                                        ip_mod, wifi_mod, zb_mod, audio_mod, \
                                        rf_mod = run.load_models(load, activity_presence)
                                        ready = 1
                                        activity_check = 0
                                    else:
                                        cprint("[LOADING] No training data available ... "
                                               "entering datastream collection mode for "
                                               + str(collect_bin) + " minute(s)", 'yellow')
                                        period = timedelta(minutes=collect_bin)
                                        training_time = datetime.now() + period
                                        ready = 0
                else:
                    if reasoning_mod:
                        load = 2
                        ip_mod, wifi_mod, zb_mod, audio_mod, \
                        rf_mod = run.load_models(load, activity_presence)
                        ready = 1
                    else:
                        print('[DEBUG] reasoning_mod: ' + str(reasoning_mod))
                        cprint("[LOADING] MAGPIE Reasoning Engine is not configured", 'yellow')
                        if train_data:
                            run.train(feed_id, activity_presence)
                            load = 2
                            ip_mod, wifi_mod, zb_mod, audio_mod, \
                            rf_mod = run.load_models(load, activity_presence)
                            ready = 1
                        else:
                            cprint("[LOADING] No training data available ... "
                                   "entering datastream collection mode for "
                                   + str(collect_bin) + " minute(s)",
                                   'yellow')
                            period = timedelta(minutes=collect_bin)
                            training_time = datetime.now() + period
                            ready = 0
            else:
                print('[DEBUG] reasoning_mod: ' + str(reasoning_mod))
                cprint("[LOADING] MAGPIE Reasoning Engine is not configured", 'yellow')
                if train_data:
                    run.train(feed_id, activity_presence)
                    load = 2
                    ip_mod, wifi_mod, zb_mod, audio_mod, \
                    rf_mod = run.load_models(load, activity_presence)
                    ready = 1
                else:
                    cprint("[LOADING] No training data available ... entering datastream collection mode for "
                           + str(collect_bin) + " minute(s)", 'yellow')
                    period = timedelta(minutes=collect_bin)
                    training_time = datetime.now() + period
                    ready = 0

        elif mode == 2:
            cprint("[LOADING] Attack testing mode ...", 'yellow')
            ready = 0
        else:
            if activity_presence == 1:
                presence = 'True'
            else:
                presence = 'False'
            cprint("[LOADING] MDS data collection mode, activity presence: " + presence, 'yellow')
            cprint("[WARNING] Collection will run for default time set in config.txt", 'yellow')
            ready = 0

        file = open("ready.txt", "w")  # hacky way of getting bash initiation script to wait until loading MDS
        file.close()
        # create zmq context
        context = zmq.Context()

        # create ZMQ TCP sockets to pull MDS feed from each Python worker script ingesting
        # stdin from the universal parser, future updates should dynamically assign ports
        consumer_receiver1 = context.socket(zmq.PULL)
        consumer_receiver2 = context.socket(zmq.PULL)
        consumer_receiver3 = context.socket(zmq.PULL)
        consumer_receiver4 = context.socket(zmq.PULL)
        consumer_receiver5 = context.socket(zmq.PULL)
        # all_connected = 0

        pipe_path = "collection_pipe"
        if not os.path.exists(pipe_path):
            os.mkfifo(pipe_path)
        pipe_fd = os.open(pipe_path, os.O_RDONLY)
        c = 0
        with os.fdopen(pipe_fd) as pipe:
            while c == 0:
                message = pipe.read()
                if message:
                    if message == '1':
                        print("[DEBUG] Received collection trigger")
                        c = 1
                    else:
                        print("[DEBUG] Received " + message)
                else:
                    print("[DEBUG] Waiting for collection trigger")
                    time.sleep(0.5)

        cprint('[LOADING] Attempting to connect MDS feeds ...', 'yellow')
        try:
            consumer_receiver1.connect("tcp://127.0.0.1:10001")  # rf feed
            consumer_receiver2.connect("tcp://127.0.0.1:10002")  # audio feed
            consumer_receiver3.connect("tcp://127.0.0.1:10003")  # zigbee feed
            consumer_receiver4.connect("tcp://127.0.0.1:10004")  # wifi feed
            consumer_receiver5.connect("tcp://127.0.0.1:10005")  # ip feed
            cprint("[READY] All MDS feeds connected", 'green')
        except:
            cprint("Error connecting to MDS ZMQ producers", 'red')

        x = 1
        first_window_skip = 0
        activity = activity_check  # Default setting
        preproc = datapreproc.feature_engineering()

        while x == 1:
            # start_time = datetime.now()
            with open('rpp.csv', 'rt', encoding='utf-8-sig') as f:
                reader = csv.reader(f)
                parameters = list(reader)
                # print(parameters)
            # parameters = [['ip', 0.1], ['wifi', 0.1], ['zigbee', 0.1], ['rf', 0.3], ['audio', 0.3], ['cutoff', 0.3]]
            cutoff = parameters[5][1]

            start_time = datetime.now()
            print('[DEBUG] Waiting to receive ZMQ MDS feeds')
            rf_feed = json.loads(consumer_receiver1.recv().decode('utf-8'))
            print('[DEBUG] RF received')
            audio_feed = json.loads(consumer_receiver2.recv().decode('utf-8'))
            print('[DEBUG] Audio received')
            zb_feed = json.loads(consumer_receiver3.recv().decode('utf-8'))
            print('[DEBUG] ZigBee received')
            wifi_feed = json.loads(consumer_receiver4.recv().decode('utf-8'))
            print('[DEBUG] WiFi received')
            ip_feed = json.loads(consumer_receiver5.recv().decode('utf-8'))
            print('[DEBUG] IP received')
            time_elapsed = datetime.now() - start_time
            print('[DEBUG]: ZMQ receive latency (ss.ms) {}'.format(time_elapsed))

            if first_window_skip == 0:
                timestamp = [int(rf_feed[0][0])]
                cprint("[DEBUG] Skipping initial MDS window: " + str(timestamp[0]), 'yellow')
                first_window_skip = 1
            else:
                start_time = datetime.now()
                timestamp = [int(rf_feed[0][0])]

                # get rid of C parser hack that provides window labels and source feed id
                # so that push_feed.py intermediate zmq producer can forward each window to magpie_main.py
                del rf_feed[1:-2]
                del audio_feed[1:-2]
                del zb_feed[1:-2]
                del wifi_feed[1:-2]
                del ip_feed[1:-2]

                for i, line in enumerate(wifi_feed):
                    del wifi_feed[i][4]
                    del wifi_feed[i][6]
                for i, line in enumerate(ip_feed):
                    del ip_feed[i][3]
                    del ip_feed[i][7]

                # aggregation of interfaces for MDS
                ip = preproc.rows("ip", ip_feed)
                wifi = preproc.rows("wifi", wifi_feed)
                zb = preproc.rows("zb", zb_feed)
                rf = preproc.rows("rf", rf_feed)
                # amds = timestamp + ip + wifi + zb + audio_feed[0][1:-2] + rf + [activity_presence]
                # del amds[0]

                if ready == 1:
                    if activity_check == 1:
                        amds = ip + wifi + zb + audio_feed[0][1:-2] + rf
                        activity_label = amds_mod.predict(amds)
                        cprint("[ACTIVITY RECOGNITION] " + str(activity), 'blue')
                        if activity_label == 0:
                            ip_mod = ip_mod1
                            wifi_mod = wifi_mod1
                            zb_mod = zb_mod1
                            rf_mod = rf_mod1
                            audio_mod = audio_mod1
                            activity_filename = "0.train"
                        else:
                            ip_mod = ip_mod2
                            wifi_mod = wifi_mod2
                            zb_mod = zb_mod2
                            rf_mod = rf_mod2
                            audio_mod = audio_mod2
                            activity_filename = "1.train"
                    else:
                        activity_label = activity_presence
                        ip_mod = ip_mod
                        wifi_mod = wifi_mod
                        zb_mod = zb_mod
                        rf_mod = rf_mod
                        audio_mod = audio_mod
                        activity_filename = (str(activity_presence) + ".train")

                    try:
                        amds = ip + wifi + zb + audio_feed[0][1:-2] + rf + [activity_label]
                        self.output(zb_h, "zigbee", zb_feed, activity_filename)
                        self.output(ip_h, "ip", ip_feed, activity_filename)
                        self.output(wifi_h, "wifi", wifi_feed, activity_filename)
                        self.output(rf_h, "rf", rf_feed, activity_filename)
                        self.output(audio_h, "audio", audio_feed, activity_filename)
                        self.output(amds_h, "amds", amds, activity_filename)
                    except:
                        print("[ERROR] Can't save to MDS datastore CSV file:", sys.exc_info()[0])

                    hvalues = self.run_feeds(parameters, ip_feed, rf_feed, wifi_feed, zb_feed, audio_feed,
                                             ip_mod, wifi_mod, zb_mod, rf_mod, audio_mod)
                    hvalues = sorted(hvalues, key=lambda x: x[1])
                    labels = ['prediction', 'feed_id']
                    df = pd.DataFrame(hvalues)
                    df.columns = labels
                    self.monitor_state(df, cutoff, rpp, start_time, mdt, timestamp, activity_label)
                else:
                    if mode == 2:
                        try:
                            activity_label = activity_presence
                            amds = ip + wifi + zb + audio_feed[0][1:-2] + rf + [activity_label]
                            self.output(zb_h, "zigbee", zb_feed, filename)
                            self.output(ip_h, "ip", ip_feed, filename)
                            self.output(wifi_h, "wifi", wifi_feed, filename)
                            self.output(rf_h, "rf", rf_feed, filename)
                            self.output(audio_h, "audio", audio_feed, filename)
                            self.output(amds_h, "amds", amds, "train")
                        except:
                            print("[ERROR] Can't save to MDS datastore CSV file:", sys.exc_info()[0]),
                        cprint('[LEARNING] Collecting training data...', 'blue')
                    else:
                        if training_time < datetime.now():
                            cprint("[RELOADING] Collection period complete", 'yellow')
                            x = 0
                            subprocess.call(['./start_magpie.sh'])
                        else:
                            activity_label = activity_presence
                            amds = ip + wifi + zb + audio_feed[0][1:-2] + rf + [activity_label]
                            try:
                                self.output(zb_h, "zigbee", zb_feed, filename)
                                self.output(ip_h, "ip", ip_feed, filename)
                                self.output(wifi_h, "wifi", wifi_feed, filename)
                                self.output(rf_h, "rf", rf_feed, filename)
                                self.output(audio_h, "audio", audio_feed, filename)
                                self.output(amds_h, "amds", amds, "train")
                            except:
                                print("[ERROR] Can't save to MDS datastore CSV file:", sys.exc_info()[0]),
                            cprint('[LEARNING] Collecting training data...', 'blue')
