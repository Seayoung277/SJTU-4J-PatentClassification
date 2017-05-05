import numpy as np
import tensorflow as tf
from random import *
import sys
import time

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
	mod = argv[1]
	pos = eval(argv[2])
	neg = eval(argv[3])
	hid = 64
	rate = 0.05
	steps = 10000
	batch = 10
	timeCost = 0

	x = []
	y = []
	w1 = []
	w2 = []
	b1 = []
	b2 = []
	p1 = []
	p2 = []
	loss = []
	optimizer = []
	train = []
	test = []

	print("Initializing ...")

	for i in range(pos*neg):
		x.append(tf.placeholder("float", shape=[None, 5000]))
		y.append(tf.placeholder("float", shape=[None]))
		w1.append(tf.Variable(tf.truncated_normal([5000, hid], stddev=0.1)))
		w2.append(tf.Variable(tf.truncated_normal([hid, 2], stddev=0.1)))
		b1.append(tf.Variable(tf.zeros(hid)))
		b2.append(tf.Variable(tf.zeros(2)))
		p1.append(tf.nn.sigmoid(tf.add(tf.matmul(x[i], w1[i]), b1[i])))
		p2.append(tf.nn.sigmoid(tf.add(tf.matmul(p1[i], w2[i]), b2[i])))
		loss.append(tf.nn.l2_loss(tf.subtract(tf.one_hot(tf.to_int32(y[i]), 2), p2[i])))
		optimizer.append(tf.train.GradientDescentOptimizer(rate))
		train.append(optimizer[i].minimize(loss[i]))
		test.append(tf.div(p2[i][0][1], p2[i][0][0]))
		#test.append(tf.argmax(p2[i], 1))
	
	init = tf.initialize_all_variables()
	sess = tf.Session()
	sess.run(init)

	print("Training ...")
	for k in range(neg):
		#print("Neg class %d" %k)
		trainData = []
		trainLabel = []
		trainCount = []
		for g in range(pos):
			(c, d, l) = readData("../" + mod + "/data/" + mod + "_train_" + repr(k*pos+g) + "_" + repr(k) + "_" + repr(g))
			trainCount.append(c)
			trainData.append(d)
			trainLabel.append(l)
		print("Batch training ...")
		for i in range(steps):
			data_dict = {}
			for j in range(pos):
				index = randint(0, trainCount[j]-batch)
				data_dict[x[k*pos+j]] = trainData[j].take(range(index, index+batch), axis=0)
				data_dict[y[k*pos+j]] = trainLabel[j].take(range(index, index+batch), axis=0)
			start = time.time()
			sess.run(train[k*pos:(k+1)*pos] + loss[k*pos:(k+1)*pos], feed_dict = data_dict)
			end = time.time()
			timeCost += (end-start)
	print("Time: %d" %(timeCost))

	print("Reading test data ...")
	(testCount, testData, testLabel) = readData("../plain/svm_test")
	
	if(argv[4]=="grid"):
		for max_th in range(neg):
			for min_th in range(pos):
				count = 0
				for i in range(1000):
					result = 0
					data_dict = {}
					for j in range(pos*neg):
						data_dict[x[j]] = testData.take([i], axis=0)
					#print("Test session run")
					vote = sess.run(test, feed_dict = data_dict)
					#print("Test session end")
					flag_max = 0
					for p in range(neg):
						flag_min = 0
						for q in range(pos):
							if(vote[p*pos+q]>1):
								flag_min += 1
						if(flag_min>min_th):
							flag_max += 1
					if(flag_max>max_th):
						result = 1
					if(result==testLabel[i]):
						count += 1
				print("Accuracy(%d, %d): %0.2f%%" %(max_th, min_th, 100.0*count/1000))
	else:
		if(mod=="rand"):
			max_th = 15
			min_th = 2
		else:
			max_th = 16
			min_th = 0
		threshod_num = 11;
		count = 0
		tp = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
		fp = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
		tn = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
		fn = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
		th = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32];
		for i in range(testCount):
			data_dict = {}
			for j in range(pos*neg):
				data_dict[x[j]] = testData.take([i], axis=0)
			#print("Test session run")
			vote = sess.run(test, feed_dict = data_dict)
			#print("Test session end")
			#print(vote)
			for j in range(threshod_num):
				result = 0
				flag_max = 0
				for p in range(neg):
					flag_min = 0
					for q in range(pos):
						if(vote[p*pos+q]>th[j]):
							flag_min += 1
					if(flag_min>min_th):
						flag_max += 1
				if(flag_max>max_th):
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
		outFile = open("m3lp_" + mod + "_test_data", 'w')
		for i in range(threshod_num):
			if((tp[i]+fn[i])!=0 and (fp[i]+tn[i])!=0 and (tp[i]+fp[i])!=0 and tp[i]!=0):
				print("TH:%0.5f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f" %(th[i], tp[i]/testCount, tn[i]/testCount, fp[i]/testCount, fn[i]/testCount, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i])))))
				outFile.write("TH:%0.5f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f" %(th[i], tp[i]/testCount, tn[i]/testCount, fp[i]/testCount, fn[i]/testCount, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i])))))
				outFile.write('\n')
		
		outFile.close()

	sess.close()

if __name__ == '__main__':
    main(sys.argv)
