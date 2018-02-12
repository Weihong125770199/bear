import Shannon as Sn
dataSet = [[1,1,'yes'],
           [1,1,'yes'],
           [1,0,'no' ],
           [0,1,'no' ],
           [0,1,'no' ]]
labels = ["No surfacing","Flippers"]
shnnonEnt = Sn.calcShannonEnt(dataSet)
#print shnnonEnt

outdataSet= Sn.splitDataSet(dataSet,1,0)
#print outdataSet

bestFeature = Sn.chooseBestFeatureToSplit(dataSet)

#print bestFeature

myTree = Sn.createTree(dataSet,labels)
print myTree
