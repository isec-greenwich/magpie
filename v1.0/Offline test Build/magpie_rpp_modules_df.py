''' NOTE:

Sklearn models must be trained on the target system and not an external system when using joblib / pickle
to save and load the model. Load a model trained on an external system will fail.

'''
import re
from sklearn.ensemble import IsolationForest
from pandas_ml import ConfusionMatrix
from sklearn.externals import joblib
import pandas as pd
import numpy as np
import csv
import math
import multiprocessing
from datetime import datetime
import ipaddress
from sklearn.metrics import accuracy_score
import os

class Model:

    def data_preproc(self, feed_id, input_data, mode):
        '''
        A function to conduct feature engineering of MDS feature vectors, where features are interpolated,
        discretisised and/or dropped
        '''

        if mode == 'train':
            data = pd.read_csv(input_data)
            data = pd.DataFrame(data)
        else:
            data = pd.DataFrame(input_data)
            timestamp = pd.DataFrame(data['timestamp'])
            timestamp['timestamp'] = pd.to_numeric(timestamp['timestamp'], downcast='integer')
            timestamps = timestamp['timestamp'].tolist()
        data = data.drop(['timestamp'], axis=1).sort_index()

        if feed_id == "ip":
            src = []
            dest = []
            direction = []
            for i in data['src']:
                if i != '0':
                    #ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                        ip = int(ipaddress.ip_address(i))
                        src.append([ip])
                    else:
                        direction.append([2])
                        #ip = int(ipaddress.ip_address("1.1.1.1"))
                        ip = int(ipaddress.ip_address(i))
                        src.append([ip])
                else:
                    ip = i
                    direction.append([0])
                    src.append([ip])

            for i in data['dest']:
                if i != '0':
                    #ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                        ip = int(ipaddress.ip_address(i))
                        dest.append([ip])
                    else:
                        direction.append([2])
                        #ip = int(ipaddress.ip_address("1.1.1.1"))
                        ip = int(ipaddress.ip_address(i))
                        dest.append([ip])
                else:
                    ip = i
                    direction.append([0])
                    dest.append([ip])


            src1 = pd.DataFrame(src, columns=['src'])
            dest1 = pd.DataFrame(dest, columns=['dest'])
            direction = pd.DataFrame(direction, columns=['direction'])

            data = data.drop(['src'], axis=1).sort_index()
            data = data.drop(['dest'], axis=1).sort_index()

            #data = pd.concat([data, src1], axis=1).sort_index()
            #data = pd.concat([data, dest1], axis=1).sort_index()


            src_port = []
            dest_port = []
            data['src_port'] = pd.to_numeric(data['src_port'])
            data['dest_port'] = pd.to_numeric(data['dest_port'])
            for i in data['src_port']:
                if i > 10000:
                    port = 50000
                else:
                    port = i
                src_port.append([port])
            for i in data['dest_port']:
                if i > 10000:
                    port = 50000
                else:
                    port = i
                dest_port.append([port])
            src_port = pd.DataFrame(src_port, columns=['src_port'])
            dest_port = pd.DataFrame(dest_port, columns=['dest_port'])

            data = data.drop(['src_port'], axis=1).sort_index()
            data = data.drop(['dest_port'], axis=1).sort_index()

            data = pd.concat([data, src_port], axis=1).sort_index()
            data = pd.concat([data, dest_port], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data['avg_del']:
                i = int(i / 1000)
                avg_del.append([i])
            for i in data['std_del']:
                i = int(i / 1000)
                std_del.append([i])
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop(['avg_del'], axis=1).sort_index()
            data = data.drop(['std_del'], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()
            data = pd.concat([data, src1], axis=1).sort_index()
            data = pd.concat([data, dest1], axis=1).sort_index()
            data['direction'] = direction['direction'].astype(int)


            #print(data)

        elif feed_id == "wifi":
            # create feature hashes for src and destination mac addresses

            src = []
            dest = []
            for i in data['src']:
                mac = int(i.replace(':', ''), 16)
                src.append([mac])
            for i in data['dest']:
                mac = int(i.replace(':', ''), 16)
                dest.append([mac])
            src1 = pd.DataFrame(src, columns=['src'])
            dest1 = pd.DataFrame(dest, columns=['dest'])

            data = data.drop(['src'], axis=1).sort_index()
            data = data.drop(['dest'], axis=1).sort_index()

            data = pd.concat([data, src1], axis=1).sort_index()
            data = pd.concat([data, dest1], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data['avg_del']:
                i = int(i / 1000)
                avg_del.append([i])
            for i in data['std_del']:
                i = int(i / 1000)
                std_del.append([i])
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop(['avg_del'], axis=1).sort_index()
            data = data.drop(['std_del'], axis=1).sort_index()

            data = data.drop(['avg_rssi'], axis=1).sort_index()
            #data = data.drop(['std_rssi'], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()

        elif feed_id == "zigbee":
            # convert zigbee src and dest addresses into discrete numbers to reduce amount of zigbee addresses
            data.loc[(data.src != '0') & (data.src != '0') & (data.src != 'ffff')
                     & (data.src != '1c7c') & (data.src != '700a'), 'src'] = '5'

            data.loc[(data.dest != '0') & (data.dest != '0000') & (data.dest != 'ffff')
                     & (data.dest != '1c7c') & (data.dest != '700a'), 'dest'] = '5'

            data.loc[(data.src == '0') & (data.dest != '0'), 'src'] = '1'
            data.loc[(data.dest == '0') & (data.src != '0'), 'dest'] = '1'
            data.loc[(data.src == 'ffff'), 'src'] = '2'
            data.loc[(data.dest == 'ffff'), 'dest'] = '2'
            data.loc[(data.src == '1c7c'), 'src'] = '3'
            data.loc[(data.dest == '1c7c'), 'dest'] = '3'
            data.loc[(data.src == '700a'), 'src'] = '4'
            data.loc[(data.dest == '700a'), 'dest'] = '4'

            data.loc[(data.src == '0') & (data.dest == '0')
                     & (data.type == 0), 'type'] = 99

            #data.loc[(data.avg_del == 0), 'avg_del'] = 1
            #data.loc[(data.std_del == 0), 'std_del'] = 1



            data['avg_sz'] = pd.to_numeric(data['avg_sz'], downcast='integer')
            data = data.drop(['std_sz'], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data['avg_del']:
                i = int(i / 1000)
                avg_del.append([i])
            for i in data['std_del']:
                i = int(i / 1000)
                std_del.append([i])
            #avg = pd.DataFrame(avg_del, columns=['avg_del'])
            #std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop(['avg_del'], axis=1).sort_index()
            data = data.drop(['std_del'], axis=1).sort_index()


            # data = data.drop(['src'], axis=1).sort_index()
            # data = data.drop(['dest'], axis=1).sort_index()
            # data = data.drop(['type'], axis=1).sort_index()

            # data = pd.concat([data, avg], axis=1).sort_index()
            # data = pd.concat([data, std], axis=1).sort_index()

            # if activity_check == 0:
            #    data = data.drop(['activity'], axis=1).sort_index()
            # else:
            #    pass

        elif feed_id == "rf":

            '''
            data = data.drop(["avg_bin14", "std_bin14", "avg_bin15", "std_bin15", "avg_bin16",
            "std_bin16", "avg_bin17", "std_bin17", "avg_bin18", "std_bin18", "avg_bin19", "std_bin19",
            "avg_bin20", "std_bin20", "avg_bin21", "std_bin21", "avg_bin22", "std_bin22", "avg_bin23",
            "std_bin23", "avg_bin24", "std_bin24",  "avg_bin25", "std_bin25","avg_bin29", "std_bin29", "avg_bin30",
            "std_bin30", "avg_bin31", "std_bin31", "avg_bin32", "std_bin32", "avg_bin33", "std_bin33",
            "avg_bin34", "std_bin34", "avg_bin35", "std_bin35", "avg_bin36", "std_bin36", "avg_bin37",
            "std_bin37", "avg_bin38", "std_bin38", "avg_bin39", "std_bin39", "avg_bin40", "std_bin40",
            "avg_bin41", "std_bin41", "avg_bin42", "std_bin42", "avg_bin43", "std_bin43", "avg_bin44",
            "std_bin44", "avg_bin45", "std_bin45", "avg_bin46", "std_bin46", "avg_bin47", "std_bin47"],
                             axis=1).sort_index()
            '''
           # print(data)
            '''
            data1 = pd.DataFrame()
            data1['avg_wifi'] = data[["avg_bin1", "avg_bin2", "avg_bin3", "avg_bin4",
                                     "avg_bin5", "avg_bin6", "avg_bin7", "avg_bin8",
                                     "avg_bin9", "avg_bin10", "avg_bin11",
                                     "avg_bin12"]].median(axis=1)

            data1['std_avg_wifi'] = data[["std_bin1", "std_bin2", "std_bin3", "std_bin4",
                                     "std_bin5", "std_bin6", "std_bin7", "std_bin8",
                                     "std_bin9", "std_bin10", "std_bin11",
                                     "std_bin12"]].median(axis=1)

            data1['avg_zigbee'] = data[["avg_bin26", "avg_bin27", "avg_bin28"]].mean(axis=1)
            data1['std_zigbee'] = data[["std_bin26", "std_bin27", "std_bin28"]].mean(axis=1)
            #'''

            #data = data1
            # print(data)
            pass

        elif feed_id == "audio":
            #data = data.drop(['std_rms'], axis=1).sort_index()
            data = data.drop(['freq'], axis=1).sort_index()
            data = data.round({'avg_rms': 0, 'std_rms': 0})
            #print(data)
            # pass
            # if activity_check == 0:
            #    data = data.drop(['activity'], axis=1).sort_index()
            # else:
            #pass
        else:
            print('feed id not found')

        # data.reset_index(drop=True, inplace=True)

        if mode == 'train':
            return data
        else:
            return data, timestamps

    def train(self, filepath, train_data, action_set):
        '''
            TRAINING FUNCTION - this functions purposes is training MAGPIE anomaly monitoring models against
            training data that consists of solely of legitimate MDS data feeds.
        '''

        def model_generator(rng, filepath, train_data, e, c, i, s):
            iforest = IsolationForest(random_state=rng, max_features=1.0, bootstrap=False,
                                      n_estimators=e, max_samples=s, contamination=c, n_jobs=-1)
            # directory to save models, feed id is for each MDS feed
            iforest_path = filepath + "models/" + train_data + "." + str(i) + ".iforest.sav"
            input_data = filepath + train_data + "." + str(i) + ".csv"
            # pre-process data before passing to model trainer
            model_data = self.data_preproc(i, input_data, 'train')  # longer term this needs to be SQLITE DB
            iforest.fit(model_data)
            joblib.dump(iforest, iforest_path)

        # random number seed for models
        rng = np.random.RandomState(67)

        # model parameters, longer term these parameters can be changed with argument variables dynamically
        # feeds = []

        # start processing delay timer
        #start_time = datetime.now()
        worker_pool = []

        # feed_id = feed_id[:-1]  # remove 'single' variable
        #for i in feed_id:
        for i in action_set[0:5]:
            if i[0] == "ip":
                c = i[2]
                e = 200
                s = 250
                # s = 50
                p = multiprocessing.Process(target=model_generator(rng, filepath, train_data, e, c, i[0], s))
                p.start()
                worker_pool.append(p)
            elif i[0] == "wifi":
                c = i[2]
                e = 200
                s = 250
                p = multiprocessing.Process(target=model_generator(rng, filepath, train_data, e, c, i[0], s))
                p.start()
                worker_pool.append(p)
            elif i[0] == "zigbee":
                c = i[2]
                # print(str(i[0]) + " " + str(c))
                e = 200
                s = 250
                p = multiprocessing.Process(target=model_generator(rng, filepath, train_data, e, c, i[0], s))
                p.start()
                worker_pool.append(p)
            elif i[0] == "rf":
                c = i[2]
                e = 200
                s = 250
                p = multiprocessing.Process(target=model_generator(rng, filepath, train_data, e, c, i[0], s))
                p.start()
                worker_pool.append(p)
            elif i[0] == "audio":
                #print(train_data)
                #if train_data == "0.train":
                #    s = 200
                #else:
                #    s = 100
                c = i[2]
                e = 200
                s = 250
                p = multiprocessing.Process(target=model_generator(rng, filepath, train_data, e, c, i[0], s))
                p.start()
                worker_pool.append(p)
            elif i == "amds":
                pass
            else:
                print("Error in processing model for feed type input")
        for p in worker_pool:
            p.join()  # Wait for all of the workers to finish.
        #print("MAGPIE model training completed and models saved with contamination:  " + str(action_set[0][2]))
        #time_elapsed = datetime.now() - start_time
        # output time taken to perform threat monitoring processing
        #print('Training Time elapsed (hh:mm:ss.ms) {}'.format(time_elapsed))

    def test(self, filepath, test_data, action_set, ip_mod,
             wifi_mod, zb_mod, rf_mod, audio_mod, risk_pref, dfb):
        '''
        TESTING FUNCTION - this functions purposes to test the trained MAGPIE anomaly monitoring models against
        test data that consists of both legitimate data and attack data. This function should be used to determine
        the detection performance of anomalies, using both the default predict and decision_function functions.

        The performance evaluation function is specifically designed to used timestamps as pseudo-labels to determine
        whether attack samples were detected outside of attack time windows.

        This function can also be used to determine general accuracy of detecting legitimate data samples.
        '''

        def performance_eval(results, test_data, risk_pref, dfb):  #
            '''
            A function to conduct model evaluation with pseudo timestamp labels
            '''
            prediction = []
            label = []
            confidence = []
            threshold = dfb

            #print(test_data)
            if test_data == "1.10train" or test_data == "0.10train":
                for i in results:
                    if risk_pref == 'TPR':
                        if i[1] >= threshold:
                        #if i[1] <= threshold:
                            # FP
                            prediction.append(1)
                            label.append(0)
                            confidence.append(round(i[1], 3))
                        elif i[1] < threshold:
                        #elif i[1] > threshold:
                            # TN
                            prediction.append(0)
                            label.append(0)
                            confidence.append(round(i[1], 3))
                        else:
                            print("Error")
                    else:
                        if i[1] <= threshold:
                            # FP
                            prediction.append(1)
                            label.append(0)
                            confidence.append(round(i[1], 3))
                        elif i[1] > threshold:
                            # TN
                            prediction.append(0)
                            label.append(0)
                            confidence.append(round(i[1], 3))
                        else:
                            print("Error")
            else:
                attack_timestamps = filepath + test_data + '.csv'
                with open(attack_timestamps, 'r') as f:
                    reader = csv.reader(f)
                    attack_times = list(reader)
                timestamp = [int(attack_times[0][0]), int(attack_times[0][1])]

                for i in results:
                    if risk_pref == 'TPR':
                        if timestamp[0] <= i[0] <= timestamp[1] and i[1] >= threshold:
                        #if timestamp[0] <= i[0] <= timestamp[1] and i[1] <= threshold:
                            # TP
                            prediction.append(1)
                            label.append(1)
                            confidence.append(round(i[1], 3))

                        elif timestamp[0] <= i[0] <= timestamp[1] and i[1] < threshold:
                        #elif timestamp[0] <= i[0] <= timestamp[1] and i[1] > threshold:
                            # FN
                            prediction.append(0)
                            label.append(1)
                            confidence.append(round(i[1], 3))

                        elif i[0] < timestamp[0] and i[1] >= threshold or i[0] > timestamp[1] and i[1] >= threshold:
                        #elif i[0] < timestamp[0] and i[1] <= threshold or i[0] > timestamp[1] and i[1] <= threshold:
                            # FP
                            prediction.append(1)
                            label.append(0)
                            confidence.append(round(i[1], 3))

                        elif i[0] < timestamp[0] and i[1] < threshold or i[0] > timestamp[1] and i[1] < threshold:
                        #elif i[0] < timestamp[0] and i[1] > threshold or i[0] > timestamp[1] and i[1] > threshold:
                            # TN
                            prediction.append(0)
                            label.append(0)
                            confidence.append(round(i[1], 3))

                        else:
                            print("Error")
                            print(timestamp, i)
                        # print(timestamp, i, test_pred, test_label)
                    else:
                        if timestamp[0] <= i[0] <= timestamp[1] and i[1] <= threshold:
                            # TP
                            prediction.append(1)
                            label.append(1)
                            confidence.append(round(i[1], 3))

                        elif timestamp[0] <= i[0] <= timestamp[1] and i[1] > threshold:
                            # FN
                            prediction.append(0)
                            label.append(1)
                            confidence.append(round(i[1], 3))

                        elif i[0] < timestamp[0] and i[1] <= threshold or i[0] > timestamp[1] and i[1] <= threshold:
                            # FP
                            prediction.append(1)
                            label.append(0)
                            confidence.append(round(i[1], 3))

                        elif i[0] < timestamp[0] and i[1] > threshold or i[0] > timestamp[1] and i[1] > threshold:
                            # TN
                            prediction.append(0)
                            label.append(0)
                            confidence.append(round(i[1], 3))

                        else:
                            print("Error")
                            print(timestamp, i)
                        # print(timestamp, i, test_pred, test_label)
            # calculate accuracy for a purely naive classifier
            naive = sum(label) / len(label)
            naive1 = 1 - naive
            if naive1 > naive:
                nc = naive1
            else:
                nc = naive

            #array = np.column_stack = (prediction, label)

            prediction = np.asarray(prediction)
            label = np.asarray(label)
            ACC = accuracy_score(label, prediction)
            # reward = np.mean(confidence)

            cubed = []
            '''
            for x in confidence:
                if presence == 1:
                    if x == 0:
                        pass
                    else:
                        cubed.append(x)
                else:
                    #cubed.append(x)
                    x = x ** 3
                    cubed.append(x)

            '''

            '''
            for x in confidence:
                if x == 0:
                    pass
                else:
                    cubed.append(x)
                    # x = x ** 3
                    # cubed.append(x)
            '''

            for x in confidence:
                x = x ** 3
                cubed.append(x)

            #'''
            cubes = sum(cubed)
            conf = sum(confidence)
            if conf == 0:
                pass
            else:
                test = (cubes / conf)
            # reward = test **(1./3.)
                conf = math.sqrt(test)
            #'''

            #conf = np.mean(confidence)

            #if risk_pref == 'TPR':
                #reward = conf

            print("Variance: " + str(round(np.var(confidence), 3)))
            reward = round(np.var(confidence), 3)
            #if conf > dfb:
            #    reward = conf
            #else:
            #    reward = 0
            #else:
                #conf = 1 - conf
                #reward = conf
                #if conf < dfb:
                #    #reward = conf
                #    reward = conf
                #else:
                #    reward = 0
            #'''

            #print('Subset Accuracy (J): ' + str(ACC))
            if test_data == "1.10train" or test_data == "0.10train":
                TPR = 'N/A'
                TNR = 'N/A'
                PREC = 'N/A'
                F1 = 'N/A'
            else:
                cm = ConfusionMatrix(prediction, label)
                stats = cm.stats()
                print(stats)
                TPR = stats['TPR']
                TNR = stats['TNR']
                PREC = stats['PPV']
                F1 = stats['F1_score']
            print('Reward: ' + str(reward))
            return reward, ACC, F1, TPR, TNR, PREC, nc, conf

        def window_sigmoid(pred, dlt):
            attack_flags = 0
            normal_flags = 0
            for i in pred:
                if i == -1:
                    attack_flags += 1
                else:
                    normal_flags += 1

            if risk_pref == 'TPR':
                x = attack_flags / (attack_flags + normal_flags)
            else:
                x = normal_flags / (attack_flags + normal_flags)
            if x == 0:
                sigmoid = [x]
            else:
                if dlt == 0:
                    sigmoid = [x]
                else:
                    sigmoid = [x / (dlt + x)]
            #sigmoid = [x]
            return sigmoid

        def run_feeds(ip_mod, zb_mod, wifi_mod, rf_mod, audio_mod, action_set):
            manager = multiprocessing.Manager()
            m_list = manager.list()
            worker_pool = []
            #print(action_set)
            for i in action_set[0:5]:  # no amds
                if i[0] == 'ip':
                    model = ip_mod
                elif i[0] == 'zigbee':
                    model = zb_mod
                elif i[0] == 'wifi':
                    model = wifi_mod
                elif i[0] == 'rf':
                    model = rf_mod
                else:
                    model = audio_mod
                p = multiprocessing.Process(target=window_process, args=(m_list, i, model))
                worker_pool.append(p)
                p.start()
            for p in worker_pool:
                p.join()  # Wait for all of the workers to finish.
            return m_list

        def output(filename, temp):
            #file_exists = os.path.isfile("/Users/mac/Dropbox/magpie/datastore/" + filename + "." + test_data + ".csv")
            outfile = open("/Users/mac/Dropbox/magpie/datastore/" + filename + test_data + ".csv", "a")
            writer = csv.writer(outfile)
            writer.writerow(temp)

        def window_process(m_list, i, model):
            #print("issue: ")
            #print(test_data)
            data = pd.read_csv(filepath + str(test_data) + "." + i[0] + ".csv")
            df = pd.DataFrame(columns=data.columns)
            window = data['timestamp'][0]  # start window timestamp
            sensitivity = i[1]
            df1 = pd.DataFrame()
            for index, row in data.iterrows():
                if row['timestamp'] == window:  # match timestamps for window
                    # print("row timestamp match")
                    df.loc[len(df)] = row
                elif row['timestamp'] != window:  # once the timestamp doesn't match process window
                    # print("window found")
                    if len(df1) == 0:
                        df1 = df
                        df = pd.DataFrame(columns=data.columns)
                        df.loc[len(df)] = row
                        window = int(row['timestamp'])
                    else:
                        # input_data = pd.concat([df1, df])  # sliding window
                        #df1 = pd.DataFrame(df)
                        #df.reset_index(drop=True, inplace=True)
                        #df1.reset_index(drop=True, inplace=True)
                        input_data = pd.concat([df1, df])
                        input_data.reset_index(drop=True, inplace=True)
                        #print(input_data)
                        model_data, timestamps = self.data_preproc(i[0], df, 'test')
                        if i[0] == 'zigbee':
                            model = zb_mod
                        elif i[0] == 'wifi':
                            model = wifi_mod
                        elif i[0] == 'ip':
                            model = ip_mod
                        elif i[0] == 'rf':
                            model = rf_mod
                        elif i[0] == 'audio':
                            model = audio_mod

                        pred = model.predict(model_data)

                        #if i[0] == 'zigbee':
                        #    pred1 = model.decision_function(model_data)
                        #    print(i[0] + " pred: " + str(pred1))

                        sig = window_sigmoid(pred, sensitivity)
                        df1 = pd.DataFrame(df)
                        time = [int(row['timestamp'])]
                        temp = time + sig + [i[0]]

                        #output(i[0], temp)
                        df = pd.DataFrame(columns=data.columns)
                        df.loc[len(df)] = row
                        window = int(row['timestamp'])
                        m_list.append(temp)

                else:
                    print("Error with timestamp")

        #start_time = datetime.now()
        hvalues = run_feeds(ip_mod, wifi_mod, zb_mod, rf_mod, audio_mod, action_set)  # run testing processes


        #time_elapsed = datetime.now() - start_time
        # output time taken to perform threat monitoring processing
        #print('Prediction Time elapsed (hh:mm:ss.ms) {}'.format(time_elapsed))
        hvalues = sorted(hvalues, key=lambda x: x[0])
        # print(hvalues)

        data = pd.read_csv(filepath + test_data + ".rf.csv")
        window = data['timestamp'][0]  # start window timestamp

        hvalue_data = pd.DataFrame(hvalues)
        hvalue_data.columns = ['timestamp', 'prediction', 'feed_id']
        h_df = pd.DataFrame(columns=hvalue_data.columns)
        hvalue_data = hvalue_data.sort_values('timestamp')


        results = []
        for index, row in hvalue_data.iterrows():
            if row['timestamp'] == window:  # match timestamps for window
                # print("row timestamp match")
                h_df.loc[len(h_df)] = row
            elif row['timestamp'] != window:  # once the timestamp doesn't match, process window
                if len(h_df) == 0:
                    # print("No sample")
                    h_df = pd.DataFrame(columns=hvalue_data.columns)
                    # initialise next window
                    h_df.loc[len(h_df)] = row
                    window = int(row['timestamp'])
                else:
                    h_df = h_df.sort_values('timestamp')
                    h_df = h_df.sort_values('feed_id')
                    hlist = []
                    print(h_df)
                    for x in range(len(h_df.index)):
                        hlist.append(h_df['prediction'].iloc[x])
                    h1 = hlist[0]
                    h2 = hlist[1]
                    h3 = hlist[2]
                    h4 = hlist[3]
                    h5 = hlist[4]
                    #sqr = math.sqrt((h2 ** 3 + h4 ** 3 + h5 ** 3)
                    #                / (h2 + h4 + h5))
                    if risk_pref == 'TPR':
                        sqr = math.sqrt((h1 ** 3 + h2 ** 3 + h3 ** 3 + h4 ** 3 + h5 ** 3)
                                        / (h1 + h2 + h3 + h4 + h5))
                    else:
                        pred_avg = sum(hlist) / len(hlist)
                        sqr = pred_avg

                    if math.isnan(sqr):
                        sqr = 0
                    else:
                        pass
                    #print(sqr)
                    # except:
                    #    sqr = 0
                    #    continue
                    time = [row['timestamp']]
                    temp = time + [sqr]
                    results.append(temp)
                    h_df = pd.DataFrame(columns=hvalue_data.columns)
                    h_df.loc[len(h_df)] = row
                    window = int(row['timestamp'])

        reward = performance_eval(results, test_data, risk_pref, dfb)
        return reward

