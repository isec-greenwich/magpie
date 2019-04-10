# from sklearn.feature_extraction import FeatureHasher
import pandas as pd
import ipaddress
import warnings
import sys
import numpy as np
if not sys.warnoptions:
    warnings.simplefilter("ignore")
import socket
import re


class feature_engineering:

    def data_preproc_train(self, feed_id, input_data):
        '''
        A function to conduct feature engineering of MDS feature vectors, where features are interpolated,
        discretisised and/or dropped
        '''

        data = pd.DataFrame(input_data)
        # w = FeatureHasher(n_features=10, input_type='string')
        data = data.drop(['timestamp'], axis=1).sort_index()

        if feed_id == "ip":
            src = []
            dest = []
            direction = []
            for i in data['src']:
                if i != '0':
                    ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                    else:
                        direction.append([2])
                else:
                    ip = i
                    direction.append([0])
                src.append([ip])
            for i in data['dest']:
                if i != '0':
                    ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                    else:
                        direction.append([2])
                else:
                    ip = i
                    direction.append([0])
                dest.append([ip])
            src1 = pd.DataFrame(src, columns=['src'])
            dest1 = pd.DataFrame(dest, columns=['dest'])
            direction = pd.DataFrame(direction, columns=['direction'])

            data = data.drop(['src'], axis=1).sort_index()
            data = data.drop(['dest'], axis=1).sort_index()

            data = pd.concat([data, src1], axis=1).sort_index()
            data = pd.concat([data, dest1], axis=1).sort_index()
            #data = pd.concat([data, direction], axis=1).sort_index()

            src_port = []
            dest_port = []
            data[data.columns[0]] = pd.to_numeric(data[data.columns[0]])
            data[data.columns[1]] = pd.to_numeric(data[data.columns[1]])
            for i in data[data.columns[0]]:
                if i > 10000:
                    port = 50000
                else:
                    port = i
                src_port.append([port])
            for i in data[data.columns[1]]:
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
            data['direction'] = direction['direction'].astype(int)

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
            src1 = pd.DataFrame(src, columns=[1])
            dest1 = pd.DataFrame(dest, columns=[2])

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

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()

        elif feed_id == "zigbee":
            # convert zigbee src and dest addresses into discrete numbers to reduce amount of zigbee addresses
            data.loc[(data.src == '0') & (data.dest != '0'), 'src'] = '1'
            data.loc[(data.dest == '0') & (data.src != '0'), 'dest'] = '1'
            data.loc[(data.src == 'ffff'), 'src'] = '2'
            data.loc[(data.dest == 'ffff'), 'dest'] = '2'
            data.loc[(data.src == '1c7c'), 'src'] = '3'
            data.loc[(data.dest == '1c7c'), 'dest'] = '3'

            data.loc[(data.src != '0') & (data.src != 'ffff')
                     & (data.src != '1c7c'), 'src'] = '4'
            data.loc[(data.dest != '0') & (data.dest != 'ffff')
                     & (data.dest != '1c7c'), 'dest'] = '4'

            data.loc[(data.src == '0') & (data.dest == '0')
                     & (data.type == 0), 'type'] = 99

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
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop(['avg_del'], axis=1).sort_index()
            data = data.drop(['std_del'], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()

            # if activity_check == 0:
            #    data = data.drop(['activity'], axis=1).sort_index()
            # else:
            #    pass

        #elif feed_id == "rf":
        #    pass
        #    data = data.drop([23 - 48], axis=1).sort_index()
        #    data = data.drop([55 - 93], axis=1).sort_index()

        #elif feed_id == 'audio':
            # data.round({'avg_rms': 0, 'std_rms': 0})
        #    pass
        else:
            pass

        return data

    def data_preproc_mon(self, feed_id, input_data):
        '''
        A function to conduct feature engineering of MDS feature vectors, where features are interpolated,
        discretisised and/or dropped
        '''

        data = pd.DataFrame(input_data)
        # w = FeatureHasher(n_features=10, input_type='string')
        data = data.drop([0], axis=1).sort_index()
        # data.reset_index(drop=True, inplace=False)

        if feed_id == "ip":

            src = []
            dest = []
            direction = []
            for i in data[data.columns[0]]:
                if i != '0':
                    # ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                        ip = int(ipaddress.ip_address(i))
                        src.append([ip])
                    else:
                        direction.append([2])
                        # ip = int(ipaddress.ip_address("1.1.1.1"))
                        ip = int(ipaddress.ip_address(i))
                        src.append([ip])
                else:
                    ip = i
                    direction.append([0])
                    src.append([ip])

            for i in data[data.columns[1]]:
                if i != '0':
                    # ip = int(ipaddress.ip_address(i))
                    pat = re.compile("^192\.168\.")
                    test = pat.match(i)
                    if test:
                        direction.append([1])
                        ip = int(ipaddress.ip_address(i))
                        dest.append([ip])
                    else:
                        direction.append([2])
                        # ip = int(ipaddress.ip_address("1.1.1.1"))
                        ip = int(ipaddress.ip_address(i))
                        dest.append([ip])
                else:
                    ip = i
                    direction.append([0])
                    dest.append([ip])

            src1 = pd.DataFrame(src, columns=[1])
            dest1 = pd.DataFrame(dest, columns=[2])
            direction = pd.DataFrame(direction, columns=['direction'])

            data = data.drop([1], axis=1).sort_index()
            data = data.drop([2], axis=1).sort_index()

            # data = pd.concat([data, src1], axis=1).sort_index()
            # data = pd.concat([data, dest1], axis=1).sort_index()

            src_port = []
            dest_port = []
            data[data.columns[0]] = pd.to_numeric(data[data.columns[0]])
            data[data.columns[1]] = pd.to_numeric(data[data.columns[1]])
            for i in data[data.columns[0]]:
                if i > 10000:
                    port = 50000
                else:
                    port = i
                src_port.append([port])
            for i in data[data.columns[1]]:
                if i > 10000:
                    port = 50000
                else:
                    port = i
                dest_port.append([port])
            src_port = pd.DataFrame(src_port, columns=['src_port'])
            dest_port = pd.DataFrame(dest_port, columns=['dest_port'])

            data = data.drop([3], axis=1).sort_index()
            data = data.drop([4], axis=1).sort_index()

            data = pd.concat([data, src_port], axis=1).sort_index()
            data = pd.concat([data, dest_port], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data[data.columns[4]]:
                i = int(float(i) / 1000)
                avg_del.append([i])
            for i in data[data.columns[5]]:
                i = int(float(i) / 1000)
                std_del.append([i])
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop([9], axis=1).sort_index()
            data = data.drop([10], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()
            data = pd.concat([data, src1], axis=1).sort_index()
            data = pd.concat([data, dest1], axis=1).sort_index()
            data['direction'] = direction['direction'].astype(int)

            print(data)

        elif feed_id == "wifi":
            # print('wifi: ')

            src = []
            dest = []
            for i in data[data.columns[0]]:
                mac = int(i.replace(':', ''), 16)
                src.append([mac])
            for i in data[data.columns[1]]:
                mac = int(i.replace(':', ''), 16)
                dest.append([mac])
            src = pd.DataFrame(src, columns=['src'])
            dest = pd.DataFrame(dest, columns=['dest'])

            data = data.drop([1], axis=1).sort_index()
            data = data.drop([2], axis=1).sort_index()

            data = pd.concat([data, src], axis=1).sort_index()
            data = pd.concat([data, dest], axis=1).sort_index()
            # data = pd.concat([data, destination], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data[data.columns[5]]:
                i = int(float(i) / 1000)
                avg_del.append([i])
            for i in data[data.columns[6]]:
                i = int(float(i) / 1000)
                std_del.append([i])
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop([8], axis=1).sort_index()
            data = data.drop([9], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()


        elif feed_id == "zigbee":
            #print('zb: ')
            #print(data)

            # convert zigbee src and dest addresses into discrete numbers to reduce amount of zigbee addresses
            data.loc[(data[data.columns[0]] == '0') & (data[data.columns[1]] != '0'), 1] = '1'
            data.loc[(data[data.columns[1]] == '0') & (data[data.columns[0]] != '0'), 2] = '1'
            data.loc[(data[data.columns[0]] == 'ffff'), 1] = '2'
            data.loc[(data[data.columns[1]] == 'ffff'), 2] = '2'
            data.loc[(data[data.columns[0]] == '1c7c'), 1] = '3'
            data.loc[(data[data.columns[1]] == '1c7c'), 2] = '3'

            data.loc[(data[data.columns[0]] != '0') & (data[data.columns[0]] != 'ffff')
                     & (data[data.columns[0]] != '1c7c'), 1] = '4'
            data.loc[(data[data.columns[1]] != '0') & (data[data.columns[1]] != 'ffff')
                     & (data[data.columns[1]] != '1c7c'), 2] = '4'

            data.loc[(data[data.columns[0]] == '0') & (data[data.columns[1]] == '0')
                     & (data[data.columns[2]] == '0'), 3] = '99'

            data[5] = pd.to_numeric(data[5], downcast='integer')
            data = data.drop([6], axis=1).sort_index()

            avg_del = []
            std_del = []
            for i in data[data.columns[5]]:
                i = int(int(float(i)) / 1000)
                avg_del.append([i])
            for i in data[data.columns[6]]:
                i = int(int(float(i)) / 1000)
                std_del.append([i])
            avg = pd.DataFrame(avg_del, columns=['avg_del'])
            std = pd.DataFrame(std_del, columns=['std_del'])

            data = data.drop([7], axis=1).sort_index()
            data = data.drop([8], axis=1).sort_index()

            data = pd.concat([data, avg], axis=1).sort_index()
            data = pd.concat([data, std], axis=1).sort_index()

        #elif feed_id == 'rf':
            # need to drop all columns except those needed for zb and wifi spectrum

            # drop 23 to 48, 55-93
            #data = data.drop([23-48], axis=1).sort_index()
            #data = data.drop([55-93], axis=1).sort_index()

        #elif feed_id == 'audio':
            # data.round({'avg_rms': 0, 'std_rms': 0})

            #pass

        else:
            pass
        return data

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
            # print(flist)
        elif feed == "zb":
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
