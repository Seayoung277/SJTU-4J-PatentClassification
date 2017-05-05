import numpy as np
import tensorflow as tf
from random import *
import sys

def readData(filename):
	print("Reading data set %s ..." %filename)
	infile = open(filename, 'r')
	count = 0
	while(infile.readline()): count += 1
	infile.seek(0, 0)

	data = np.zeros((count, 5000))
	label = np.zeros((count,))
	cnt = 0
	line = infile.readline()
	while(line != ''):
		p1 = line.find(" ")
		label[cnt] = eval(line[0:p1])
		p3 = line.find(" ", p1+1)
		while(p3>=0):
			p2 = line.find(":", p1+1)
			data[cnt][eval(line[p1+1:p2])-1] = eval(line[p2+1:p3])
			p1 = p3
			p3 = line.find(" ", p1+1)
		cnt += 1
		line = infile.readline()
	infile.close()
	return count, data, label
	
def main(argv):
	hid = 64
	rate = 0.05
	steps = 30000
	batch = 10

	print("Initializing ...")

	x = tf.placeholder("float", shape=[None, 5000])
	y = tf.placeholder("float", shape=[None])
	w1 = tf.Variable(tf.truncated_normal([5000, hid], stddev=0.1))
	w2 = tf.Variable(tf.truncated_normal([hid, 2], stddev=0.1))
	b1 = tf.Variable(tf.zeros(hid))
	b2 = tf.Variable(tf.zeros(2))
	p1 = tf.nn.sigmoid(tf.matmul(x, w1)+b1)
	p2 = tf.nn.sigmoid(tf.matmul(p1, w2)+b2)
	loss = tf.nn.l2_loss(tf.one_hot(tf.to_int32(y), 2)-p2)
	optimizer = tf.train.GradientDescentOptimizer(rate)
	train = optimizer.minimize(loss)
	test = tf.div(p2[0][1], p2[0][0])
	
	init = tf.initialize_all_variables()
	sess = tf.Session()
	sess.run(init)

	print("Training ...")
	(trainCount, trainData, trainLabel) = readData("../plain/svm_train")
	for i in range(steps):
		data_dict = {}
		index = randint(0, trainCount-batch)
		sess.run([train, loss], feed_dict = {x:trainData.take(range(index, index+batch), axis=0), y:trainLabel.take(range(index, index+batch), axis=0)})

	print("Reading test data ...")
	(testCount, testData, testLabel) = readData("../plain/svm_test")
	count = 0;
	threshod_num = 11;
	tp = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
	fp = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
	tn = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
	fn = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
	th = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32];
	for i in range(testCount):
		vote = sess.run(test, feed_dict = {x:testData.take([i], axis=0)})
		for j in range(threshod_num):
			result = 0
			if(vote>th[j]):
				result = 1
			if(j==5 and testLabel[i]==result):
				count += 1
			if(testLabel[i]==1 and result==1):
				tp[j] += 1.
			elif(testLabel[i]==1 and result==0):
				fn[j] += 1.
			elif(testLabel[i]==0 and result==1):
				fp[j] += 1.
			elif(testLabel[i]==0 and result==0):
				tn[j] += 1.
	print("Accuracy: %0.4f" %(1.0*count/testCount))
	outFile = open("mlp_test_data", 'w')
	for i in range(threshod_num):
		if((tp[i]+fn[i])!=0 and (fp[i]+tn[i])!=0 and (tp[i]+fp[i])!=0 and tp[i]!=0):
			print("TH:%0.5f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f" %(th[i], tp[i]/testCount, tn[i]/testCount, fp[i]/testCount, fn[i]/testCount, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i])))))
			outFile.write("TH:%0.5f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f" %(th[i], tp[i]/testCount, tn[i]/testCount, fp[i]/testCount, fn[i]/testCount, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i])))))
			outFile.write('\n')
		
	outFile.close()
	sess.close()

if __name__ == '__main__':
    main(sys.argv)
