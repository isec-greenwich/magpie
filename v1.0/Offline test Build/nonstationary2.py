from pprint import pprint
import magpie_rpp_modules_df as magpie
import random
import bisect
import numpy as np
from sklearn.externals import joblib

class WeightedTuple(object):

    def __init__(self, items):
        self.indexes = []
        self.items = []
        next_index = 0
        for key in sorted(items.keys()):
            val = items[key]
            self.indexes.append(next_index)
            self.items.append(key)
            next_index += val

        self.len = next_index

    def __getitem__(self, n):
        if n < 0:
            n = self.len + n
        if n < 0 or n >= self.len:
            raise IndexError

        idx = bisect.bisect_right(self.indexes, n)
        return self.items[idx - 1]

    def __len__(self):
        return self.len


def testAlgorithms(action_set, filepath, test_data,
                   algorithms, numberOfRuns, numberOfPulls, isStationary,
                   train, feeds, train_file, risk_pref, dfb):

    # Set up the structures for storing the algorithms results

    # Dictionary: Keys are the algorithm object. Value is an array of rewards for
    # each pull 0->numberOfPulls, averaged over numberOfRuns
    algorithmRewards = {}

    # Dictionary: Keys are the algorithm object. Value is an array of %'s where
    # optimal action was taken for each pull 0->numberOfPulls, averaged over numberOfRuns
    algorithmOptimals = {}

    for algorithm in algorithms:
        returns = [0.0] * numberOfPulls
        algorithmRewards[algorithm] = returns

        optimals = [0.0] * numberOfPulls
        algorithmOptimals[algorithm] = optimals

    #data = WeightedTuple({test_data[0][0]: 12, test_data[0][1]: 12, test_data[0][2]: 12, test_data[0][3]: 12,
    #                        test_data[0][4]: 12, test_data[0][5]: 12, test_data[0][6]: 12, test_data[0][7]: 12})
    data = WeightedTuple({test_data[0][0]: 25, test_data[0][1]: 75})
    test = random.choice(data)
    print("Sample: " + str(test))
    # rpp_selected = []

    for run in range(numberOfRuns):
        if run % 1 == 0:
            print("Executing run " + str(run) + " ... ")
        bandit = magpie.Model()
        for pull in range(numberOfPulls):
            # Walk the bandit if necessary
            print("Sample: " + str(test))
            for algorithm in algorithms:
                # ask algorithm for the arm it should pull
                armIndex = algorithm.policy()
                arm = action_set[armIndex]
                pprint("Arm Index: " + str(armIndex))
                pprint("Arm: " + str(arm))
                pull = pull + 1
                pprint("Pull: " + str(pull))
                pull = pull - 1
                # pull the arm and collect the reward
                # reward = bandit.pull(arm)

                bandit.train(filepath, train, arm)

                ip_mod = joblib.load(filepath + "models/" + train + ".ip.iforest.sav")
                wifi_mod = joblib.load(filepath + "models/" + train + ".wifi.iforest.sav")
                zb_mod = joblib.load(filepath + "models/" + train + ".zigbee.iforest.sav")
                rf_mod = joblib.load(filepath + "models/" + train + ".rf.iforest.sav")
                audio_mod = joblib.load(filepath + "models/" + train + ".audio.iforest.sav")

                reward, ACC, F1, TPR, TNR, PREC, nc, conf = bandit.test(filepath, test, arm, ip_mod, wifi_mod, zb_mod,
                                     rf_mod, audio_mod, risk_pref, dfb)

                print("Reward: " + str(reward))
                # allow the algorithm to learn based on result of the arm
                Q = algorithm.learn(reward, armIndex)

                # update the statistics for the algorithm
                rewardArray = algorithmRewards[algorithm]
                optimalActionArray = algorithmOptimals[algorithm]
                optimalAction = 0

                stepSize = 1 / (run + 1)

                if reward > np.argmax(rewardArray):
                    optimalAction = 1
                else:
                    optimalAction = 0

                rewardArray[pull] += stepSize * (reward - rewardArray[pull])
                optimalActionArray[pull] += stepSize * (optimalAction - optimalActionArray[pull])

                # walk the bandit
                if not isStationary:
                    test = random.choice(data)

    # return the dictionaries of learning results as a tuple
    learning_results = ({'rewards': algorithmRewards, 'optimalActions':algorithmOptimals})
    return Q, learning_results
