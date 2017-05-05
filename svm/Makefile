CXX ?= g++
CC ?= gcc
CFLAGS = -Wconversion -O3 -fPIC
LIBS = ../liblinear-2.1/blas/blas.a

all: svm data m3 m3mp

data: data.cpp
	$(CXX) $(CFLAGS) -o data data.cpp

svm: svm.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o ../liblinear-2.1/blas/blas.a
	$(CXX) $(CFLAGS) -o svm svm.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o $(LIBS)

m3: m3.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o ../liblinear-2.1/blas/blas.a
	$(CXX) $(CFLAGS) -o m3 m3.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o $(LIBS)

m3mp: m3mp.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o ../liblinear-2.1/blas/blas.a
	$(CXX) $(CFLAGS) -fopenmp -o m3mp m3mp.cpp ../liblinear-2.1/tron.o ../liblinear-2.1/linear.o $(LIBS)
