import numpy as np


class UCB:
    # eps of -1 will use an average reward (1/n) rather than a constant step size
    def __init__(self, action_space, c, alpha):
        self.name = "UCB"
        # self.bandit = bandit
        self.numberOfArms = action_space
        self.numberOfPullsArray = [0] * self.numberOfArms
        self.numberOfPulls = 0

        self.Q = [0.0] * self.numberOfArms
        self.c = c
        self.alpha = alpha

    def learn(self, reward, armIndex):
        # Update the Action values
        #print(armIndex)
        self.numberOfPullsArray[armIndex] += 1
        self.numberOfPulls += 1
        if (self.alpha == -1):
            stepSize = 1 / self.numberOfPullsArray[armIndex]
        else:
            stepSize = self.alpha

        # do the learning
        # print("Learning with reward: " + str(reward) + " armIndex: " + str(armIndex))
        # print("Q before learning: ")
        # print(self.Q)
        self.Q[armIndex] += stepSize * (reward - self.Q[armIndex])

        #print(self.Q)
        return self.Q

    def policy(self):
        # armIndex = 0
        # Upper confidence bound action selection
        # A=argmax(Q(a)+c*(sqrt(log(t)/N(a))
        A = []
        currentBestArm = np.argmax(self.Q)
        for i in range(self.numberOfArms):
            if self.numberOfPullsArray[i] > 0:
                val = self.Q[i] + self.c * np.sqrt((np.log(self.numberOfPulls) / self.numberOfPullsArray[i]))
            else:
                # We want this to be a maximizing action
                val = 10000
            if i == currentBestArm:
            # if (False):
                # We never want to "explore" the best arm. So set it's A value to very low
                A.append(-10000)
            else:
                A.append(val)

        armIndex = np.argmax(A)
        return armIndex

    # def reset(self):
    #    self.Q = [0.0] * self.numberOfArms
    #    self.numberOfPullsArray = [0] * self.numberOfArms
    #    self.numberOfPulls = 0