''' NOTE:

Sklearn models must be trained on the target system and not an external system when using joblib / pickle
to save and load the model. Load a model trained on an external system will fail.

'''
import pandas as pd
import numpy as np
import multiprocessing
import socket
from functools import reduce
import operator
import csv
import os

class Model:

    def output(self, header, feed_window, filename):
        file_exists = os.path.isfile("/Users/mac/Dropbox/magpie/datastore/" + filename + ".amds_test.csv")
        outfile = open("/Users/mac/Dropbox/magpie/datastore/" + filename + ".amds_test.csv", "a")
        writer = csv.writer(outfile)
        if not file_exists:
            writer.writerow(header)
        writer.writerow(feed_window)

    def rows(self, feed, lines):
        # subnet could be added automatically as a input parameter in startup script if required
        ip_min, ip_max = socket.inet_aton('192.168.1.0'), socket.inet_aton('192.168.1.255')

        avg_freq, avg_sz, std_sz, avg_delay, std_delay, avg_rssi, std_rssi, result, \
        ftype, std_db, avg_db, avg_d, rf_db, flist = [], [], [], [], [], [], [], [], [], [], [], [], [], []

        if feed == "ip":
            avg_ttl = []
            # std_ttl = []
            for i in lines:
                if ip_min < socket.inet_aton(str(i[1])) < ip_max:
                    for t in range(1, int(i[5]) + 1):
                        port = int(i[4])
                        if port > 10000:
                            port = 50000
                        else:
                            pass
                        ftype.append(port)
                    else:
                        port = int(i[3])
                        if port > 10000:
                            port = 50000
                        else:
                            pass
                        ftype.append(port)
                avg_freq.append(int(i[5]))  # at the moment these are just averages
                avg_ttl.append(int(float(i[6])))
                # std_ttl.append(int(float(i[8])))
                avg_sz.append(int(float(i[7])))
                std_sz.append(int(float(i[8])))
                avg_delay.append(int(float(i[9]) / 1000))
                std_delay.append(int(float(i[10]) / 1000))
            flist = [ftype, avg_freq, avg_sz, std_sz, avg_ttl, avg_delay, std_delay]

        elif feed == "wifi":
            for i in lines:
                ftype.append(int(i[4]))
                avg_freq.append(int(float(i[3])))  # at the moment these are just averages
                avg_sz.append(int(float(i[5])))
                # std_sz.append(int(float(i[7])))
                avg_rssi.append(int(float(i[6])))
                std_rssi.append(int(float(i[7])))
                avg_delay.append(int(float(i[8]) / 1000))
                std_delay.append(int(float(i[9]) / 1000))
            flist = [ftype, avg_freq, avg_sz, avg_rssi, std_rssi, avg_delay, std_delay]
        elif feed == "zigbee":
            for i in lines:
                ftype.append(int(i[3]))
                avg_freq.append(int(i[4]))  # at the moment these are just averages
                avg_sz.append(int(float(i[5])))
                std_sz.append(int(float(i[6])))
                avg_delay.append(int(float(i[7]) / 1000))
                std_delay.append(int(float(i[8]) / 1000))
            flist = [ftype, avg_freq, avg_sz, std_sz, avg_delay, std_delay]

        elif feed == "rf":
            for i in lines[0][1:-2]:
                rf_db.append(float(i))
            avg_db = int(np.nanmean(rf_db))
            std = np.std(rf_db)
            std_db = float(np.round(std, 2))

        else:
            print("Error: unknown field type in aMDS 'rows' function")

        # needed to skip all the other processing of other feed types
        if feed == "rf":
            result.append(avg_db)
            result.append(std_db)
        else:
            flist.append(np.std(flist[1]))  # create std for freq feature where there are multiple packets per window
            result.append(np.sum(flist[1]))
            result.append(int(np.sum(flist[2])))
            try:
                result.append(max(flist[0], key=lambda x: flist[0].count(x)))
                result.append(min(flist[0], key=lambda x: flist[0].count(x)))
            except:
                result.append(0)
                result.append(0)
            del flist[0]
            for i in flist:
                result.append(int(np.nanmean(i)))  # ignore nan values which will cause an exception
        return result

    def test(self, filepath, test_data, action_set, label):

        def run_feeds(action_set):
            manager = multiprocessing.Manager()
            m_list = manager.list()
            worker_pool = []
            for i in action_set[0:5]:  # no amds
                p = multiprocessing.Process(target=window_process, args=(m_list, i))
                worker_pool.append(p)
                p.start()
            for p in worker_pool:
                p.join()  # Wait for all of the workers to finish.
            return m_list

        def window_process(m_list, i):
            data = pd.read_csv(filepath + str(test_data) + "." + i[0] + ".csv")
            df = pd.DataFrame(columns=data.columns)
            window = data['timestamp'][0]  # start window timestamp
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
                        df = pd.DataFrame(columns=data.columns)
                        df.loc[len(df)] = row
                        window = int(row['timestamp'])
                        df_list = df.values.tolist()
                        if i[0] == 'ip':
                            tlist = self.rows("ip", df_list)
                        elif i[0] == 'zigbee':
                            tlist = self.rows("zigbee", df_list)
                        elif i[0] == 'audio':
                            del df_list[0][0]
                            tlist = df_list
                            tlist = reduce(operator.concat, tlist)
                        elif i[0] == 'wifi':
                            tlist = self.rows("wifi", df_list)
                        else:
                            tlist = self.rows("rf", df_list)
                        temp = [i[0]] + [window] + tlist
                        m_list.append(temp)


                else:
                    print("Error with timestamp")

        m_amds = run_feeds(action_set)  # run testing processes

        #print(m_amds)
        ip, wifi, zigbee, rf, audio, amds_c, window = [], [], [], [], [], [], []

        for i in m_amds:
            window.append(i[1])

        for i in m_amds:
            if i[0] == 'audio':
                audio.append(i[1:])
            elif i[0] == 'ip':
                ip.append(i[1:])
            elif i[0] == 'wifi':
                wifi.append(i[1:])
            elif i[0] == 'zigbee':
                zigbee.append(i[1:])
            elif i[0] == 'rf':
                rf.append(i[1:])

        amds_h = ["ip_sum_freq", "ip_sum_sz", "ip_destp_mode", "ip_destp_least", "ip_avg_freq",
                  "ip_avg_sz", "ip_std_sz", "ip_avg_ttl", "ip_avg_delay", "ip_std_delay", "ip_std_freq",
                  "wifi_sum_freq", "wifi_sum_sz", "wifi_subtype_mode", "wifi_type_least", "wifi_avg_freq",
                  "wifi_avg_sz", "wifi_avg_rssi", "wifi_std_rssi", "wifi_avg_delay", "wifi_std_delay",
                  "wifi_std_freq",
                  "zb_sum_freq", "zb_sum_sz", "zb_type_mode", "zb_type_least", "zb_avg_freq", "zb_avg_sz",
                  "zb_std_sz", "zb_avg_delay", "zb_std_delay", "zb_std_freq",
                  "audio_freq", "audio_avg_rms", "audio_std_rms", "avg_db", "std_db", 'activity']

        rf_df = pd.DataFrame(rf).sort_values(by=[0])
        ip_df = pd.DataFrame(ip).sort_values(by=[0])
        wifi_df = pd.DataFrame(wifi).sort_values(by=[0])
        zigbee_df = pd.DataFrame(zigbee).sort_values(by=[0])
        audio_df = pd.DataFrame(audio).sort_values(by=[0])

        merged_df = ip_df.merge(wifi_df, how='inner', on=[0])
        merged_df1 = merged_df.merge(zigbee_df, how='inner', on=[0])
        merged_df2 = merged_df1.merge(audio_df, how='inner', on=[0])
        amds = merged_df2.merge(rf_df, how='inner', on=[0])
        amds.drop([0], axis=1, inplace=True)
        amds['Name'] = label
        print(amds)
        amds_ls = amds.values.tolist()
        del amds_ls[0]
        for i in amds_ls:
            self.output(amds_h, i, test_data)




