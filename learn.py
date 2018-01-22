"""
============================================================================
Comparing anomaly detection algorithms for outlier detection on toy datasets
============================================================================

This example shows characteristics of different anomaly detection algorithms
on 2D datasets. Datasets contain one or two modes (regions of high density)
to illustrate the ability of algorithms to cope with multimodal data.

For each dataset, 15% of samples are generated as random uniform noise. This
proportion is the value given to the nu parameter of the OneClassSVM and the
contamination parameter of the other outlier detection algorithms.
Decision boundaries between inliers and outliers are displayed in black.

Local Outlier Factor (LOF) does not show a decision boundary in black as it
has no predict method to be applied on new data.

While these examples give some intuition about the algorithms, this
intuition might not apply to very high dimensional data.

Finally, note that parameters of the models have been here handpicked but
that in practice they need to be adjusted. In the absence of labelled data,
the problem is completely unsupervised so model selection can be a challenge.
"""

# Author: Alexandre Gramfort <alexandre.gramfort@inria.fr>
#         Albert Thomas <albert.thomas@telecom-paristech.fr>
# License: BSD 3 clause

import time

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from sklearn import svm
from sklearn.datasets import make_moons, make_blobs
from sklearn.covariance import EllipticEnvelope
from sklearn.ensemble import IsolationForest
from sklearn.neighbors import LocalOutlierFactor
X = np.array([1,2,3,4])
A = np.array([[1,1,1,1],[2,2,2,2],[3,3,3,3],[4,4,4,4]])
Y = np.dot(A,X)
twodAx = plt.subplot(221)
twodAx.scatter(X,Y)
twodAx.xlable = ("X")
twodAx.xlable = ("Y")
twodAx.scatter(X,Y)
twodAx.scatter(X,X,c = 'r')
threedAx=plt.subplot(222,projection = '3d')
p = np.array([[0,0,0],[1,0,0],[0,1,0],[1,1,1]])
threedAx.scatter(p.T[0],p.T[1],p.T[2])
threedAx.set_xlabel("X")
threedAx.set_ylabel("Y")
threedAx.set_zlabel("Z")
fig = plt.figure(2)
surface =  Axes3D(fig)
X=np.arange(1,10,1) 
Y=np.arange(1,20,1) 
X, Y = np.meshgrid(X, Y) 
Z = X**3 + Y**2 
surface.plot_surface(X,Y,Z)
plt.show()
