import pandas as pd

# Import train_test_split function
from sklearn.model_selection import train_test_split
from sklearn.cross_validation import cross_val_score
import numpy as np
data = pd.read_csv("/Users/mac/Dropbox/magpie/datastore/activity_presence.amds.csv")
from sklearn.externals import joblib
#'''
X=data[['ip_sum_freq','ip_sum_sz','ip_avg_freq','ip_avg_sz','ip_std_sz','ip_std_freq',
        'wifi_sum_freq','wifi_sum_sz','wifi_subtype_mode','wifi_type_least',
        'wifi_avg_freq','wifi_avg_sz', 'wifi_std_freq',
        'zb_sum_freq','zb_sum_sz','zb_type_mode','zb_type_least',
        'zb_avg_freq','zb_avg_sz','zb_std_sz','zb_std_freq']]  # Features
#'''
'''
X=data[['ip_avg_freq','ip_std_freq','wifi_sum_sz','wifi_subtype_mode','wifi_type_least',
        'wifi_std_freq',
        'zb_sum_freq','zb_sum_sz','zb_type_mode','zb_type_least',
        'zb_avg_freq','zb_std_sz','zb_std_freq',
        'audio_freq','audio_avg_rms','audio_std_rms','avg_db','std_db']]  # Features
'''
#X.drop(columns=['activity'])
y=data['activity']  # Labels

# Split dataset into training set and test set
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3) # 70% training and 30% test

from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import cross_val_score

param_grid = {
                 'n_estimators': [50, 100, 150, 200],
                 'max_depth': [2, 5, 7, 9, 11, 15, 20]
             }

#Create a Gaussian Classifier
clf=RandomForestClassifier(bootstrap=False, class_weight=None, criterion='gini',
            max_depth=None, max_features='auto', max_leaf_nodes=None,
            min_impurity_decrease=0.0, min_impurity_split=None,
            min_samples_leaf=1, min_samples_split=2,
            min_weight_fraction_leaf=0.0, n_estimators=100, n_jobs=-1,
            oob_score=False, random_state=None, verbose=0,
            warm_start=False)

#Train the model using the training sets y_pred=clf.predict(X_test)
#clf.fit(X_train,y_train)



from sklearn.grid_search import GridSearchCV

grid_clf = GridSearchCV(clf, param_grid, cv=10, scoring='f1')
grid_clf.fit(X_train, y_train)

print(grid_clf.best_estimator_)
print(grid_clf.best_params_)
print(grid_clf.grid_scores_)


scores = cross_val_score(grid_clf.best_estimator_, X_train, y_train, cv=10, scoring='roc_auc')
print("Accuracy: %0.2f (+/- %0.2f)" % (scores.mean(), scores.std() * 2))

import time

start_time = time.time()
y_pred = grid_clf.best_estimator_.predict(X_test)
elapsed_time = time.time() - start_time
print(elapsed_time)
from sklearn import metrics
# Model Accuracy, how often is the classifier correct?
print("Accuracy:",metrics.accuracy_score(y_test, y_pred))
print("F1:",metrics.f1_score(y_test, y_pred))
print("Prec:",metrics.precision_score(y_test, y_pred))
print("Report:",metrics.classification_report(y_test, y_pred))
feature_imp = pd.Series(grid_clf.best_estimator_.feature_importances_,index=X.columns.sort_values(ascending=True))
print(feature_imp)

#probability = grid_clf.best_estimator_.predict_proba(X_test)[:,1]
#classification = grid_clf.best_estimator_.predict(X_test)[:,1]
#print(probability)
#print(classification)
joblib.dump(grid_clf.best_estimator_, "/Users/mac/Dropbox/magpie/datastore/rf_test_nophys.sav")