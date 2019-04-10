''' NOTE:

Sklearn models must be trained on the target system and not an external system when using joblib / pickle
to save and load the model. Load a model trained on an external system will fail.

'''
from sklearn.ensemble import IsolationForest
from sklearn.externals import joblib
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split, GridSearchCV
import pandas as pd
import numpy as np
import multiprocessing
from datetime import datetime
import magpie_datapreproc as datapreproc
from termcolor import cprint
import warnings
import sys

if not sys.warnoptions:
    warnings.simplefilter("ignore")



class Model:

    def load_models(self, load, activity_presence):
        if load == 1:
            ip_mod1 = joblib.load("/home/pi/magpie/models/0.reasoning.ip.sav")
            wifi_mod1 = joblib.load("/home/pi/magpie/models/0.reasoning.wifi.sav")
            zb_mod1 = joblib.load("/home/pi/magpie/models/0.reasoning.zigbee.sav")
            audio_mod1 = joblib.load("/home/pi/magpie/models/0.reasoning.audio.sav")
            rf_mod1 = joblib.load("/home/pi/magpie/models/0.reasoning.rf.sav")
            ip_mod2 = joblib.load("/home/pi/magpie/models/1.reasoning.ip.sav")
            wifi_mod2 = joblib.load("/home/pi/magpie/models/1.reasoning.wifi.sav")
            zb_mod2 = joblib.load("/home/pi/magpie/models/1.reasoning.zigbee.sav")
            audio_mod2 = joblib.load("/home/pi/magpie/models/1.reasoning.audio.sav")
            rf_mod2 = joblib.load("/home/pi/magpie/models/1.reasoning.rf.sav")
            #amds_mod = joblib.load("/home/pi/magpie/models/activity_recog.amds.sav")
            amds_mod = joblib.load("/home/pi/magpie/datastore/rf.sav")
            cprint("[COMPLETE] MAGPIE Reasoning Engine ready!", 'green')
            return ip_mod1, wifi_mod1, zb_mod1, audio_mod1, rf_mod1, \
                   ip_mod2, wifi_mod2, zb_mod2, audio_mod2, rf_mod2, amds_mod
        else:
            ip_mod = joblib.load("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.ip.sav")
            wifi_mod = joblib.load("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.wifi.sav")
            zb_mod = joblib.load("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.zigbee.sav")
            audio_mod = joblib.load("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.audio.sav")
            rf_mod = joblib.load("/home/pi/magpie/models/" + str(activity_presence) + ".reasoning.rf.sav")
            cprint("[COMPLETE] MAGPIE Reasoning Engine ready!", 'green')
            return ip_mod, wifi_mod, zb_mod, audio_mod, rf_mod


    def train(self, feed_id, activity_presence):
        '''
            TRAINING FUNCTION - this functions purposes is training MAGPIE anomaly monitoring models against
            training data that consists of solely of legitimate MDS data feeds. Future iterations of MAGPIE
            will be designed to cope with adversarial feeds (e.g., where training data already contains attack
            outlier data
        '''

        def model_generator(e, c, i, s):
            preproc = datapreproc.feature_engineering()
            # random number seed for models
            rng = np.random.RandomState(100)
            if activity_presence == 1:
                filename = "1.train"
            else:
                filename = "0.train"
            if i == 'amds':
                data = pd.read_csv("/home/pi/magpie/datastore/train.amds.csv")
                X = data.drop(['activity'], axis=1).sort_index()
                y = data['activity']  # Labels

                X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3)  # 70% train 30% test
                rf = RandomForestClassifier(bootstrap=True, class_weight=None, criterion='gini',
                                             max_depth=15, max_features='auto', max_leaf_nodes=None,
                                             min_samples_leaf=1, min_samples_split=2,
                                             n_estimators=150, n_jobs=-1, random_state=rng)
                rf.fit(X_train, y_train)
                joblib.dump(rf, "/home/pi/magpie/models/activity_recog." + str(i) + ".sav")
            else:
                input_data = pd.read_csv("/home/pi/magpie/datastore/" + filename + "." + str(i) + ".csv")
                model_data = preproc.data_preproc_train(i, input_data)

                iforest = IsolationForest(random_state=rng, n_estimators=e,
                                            max_samples='auto', contamination=c, n_jobs=-1)
                iforest.fit(model_data)
                joblib.dump(iforest, "/home/pi/magpie/models/" + str(activity_presence)
                            + ".reasoning." + str(i) + ".sav")

        def model_config(e, c, i, s):
            model = multiprocessing.Process(target=model_generator(e, c, i, s))
            return model

        cprint("[LEARNING] MAGPIE is baselining the smart home, please wait ...",
               'magenta')

        start_time = datetime.now()
        worker_pool = []

        for i in feed_id:
            if i == "ip":
                e = 200
                s = 100
                c = 0.01
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            elif i == "wifi":
                e = 300
                s = 25
                c = 0.001
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            elif i == "zigbee":
                e = 300
                s = 100
                c = 0.1
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            elif i == "rf":
                e = 150
                c = 0.05
                s = 25
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            elif i == "audio":
                e = 150
                c = 0.005
                s = 50
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            elif i == "amds":
                e = 0   # NA
                c = 0   # NA
                s = 0   # NA
                p = model_config(e, c, i, s)
                p.start()
                worker_pool.append(p)
            else:
                print("Error in processing model for feed type input")
        for p in worker_pool:
            p.join()  # Wait for all of the workers to finish.
        time_elapsed = datetime.now() - start_time
        cprint("[COMPLETE] MAGPIE learning successful", 'green')
        cprint("[INFORMATION] Learning time taken: (hh:mm:ss.ms) {}".format(time_elapsed), 'green')
        return
