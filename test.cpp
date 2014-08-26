#include <iostream>
#include <vector>
#include <time.h>
#include "armadillo"
#include <float.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
using namespace arma;

class Jacobian{

  public:

  void kmeans( vector<double>* Cluster_index, vector<double*>* Cluster_center,double **Data, int *Output, double** Cluster_Center, int* trainExpect, int data_number, int data_dimension, int Center_Number, int categoryNum,int MaxIteration);


double LMSTrain( vector< vector <double> > *J_hidden, vector<double>* Cluster_index, vector<double*>* Cluster_center, double** Data, double** Cluster_Center, int* trainExcept, int* Output, int data_number, int data_dimension, int Center_Number, int categoryNum,  char* richtextbox1, int MaxIteration);

void Discrete_Output(vector<double>* J_Output, int categoryNum);


void ReadFile(char* filename,vector< vector <double> >* trainData,vector< vector<double> > *testData, vector<int>* trainExcept,vector<int>* testExcept,int* train_data_number,int* test_data_number,int* data_dimension,int* categoryNum);


double Test(vector< vector <double> > J_hidden, vector<double> Cluster_index, vector<double*> Cluster_Center, double** Data, int* testExpect, int data_number, int data_dimension, int categoryNum);
};


double Jacobian::Test(vector< vector<double> >J_hidden, vector<double> Cluster_index, vector<double*> Cluster_Center, double** Data, int* testExpect, int data_number, int data_dimension, int categoryNum)
        {
            vector<double> J_Output;// = new List<double>();
            for (int i = 0; i < data_number; i++)
            {
                double mmin = DBL_MAX;//double.MaxValue;//;
                int index = 0;
                for (int k = 0; k < Cluster_index.size(); k++)
                {
                    double dis = 0;
                    for (int d = 0; d < data_dimension; d++)
                        dis += (Data[i][d] - Cluster_Center[k][d]) * (Data[i][d] - Cluster_Center[k][d]);
                    dis = sqrt(dis);
                   if (dis < mmin)
                   {
                       mmin = dis;
                       index = k;
                   }
                }
                double* array = new double[data_dimension];
                for (int d = 0; d < data_dimension; d++) array[d] = Data[i][d] - Cluster_Center[index][d];
                double tmp = 0;
                //double[][,] A=J_hidden.ToArray();
                for (int d = 0; d < data_dimension; d++) tmp += array[d] * J_hidden[index][d];
                J_Output.push_back(tmp + Cluster_index[index]);

                delete[] array;
            }

            Discrete_Output(&J_Output, categoryNum);
            double correct = 0;
            for (int i = 0; i < data_number; i++)
            {
                if (J_Output[i] == testExpect[i])
                {
                    correct++;
                }
            }
            correct = correct / J_Output.size();

            return correct;
        }



void Jacobian::ReadFile(char* filename,vector< vector <double> >* trainData,vector< vector<double> > *testData, vector<int>* trainExcept,vector<int>* testExcept,int* train_data_number,int* test_data_number,int* data_dimension,int* categoryNum)
{
  //char line[];


  vector< vector<double> > data;
  std::ifstream fin(filename);
  std::string line;
  while(std::getline(fin,line)){
    std::stringstream ss(line);
    vector<double> row;
    double tmp;
    while(ss>>tmp){
       row.push_back(tmp);
    }
    data.push_back(row);
  }

  int data_number=data.size();
  *data_dimension=(int)data[0].size();
  *data_dimension=*data_dimension-1;
  int D_dim = *data_dimension;
  int train_num = *train_data_number = data_number/2;
  int test_num = *test_data_number = data_number-train_num;


  double **Data = new double*[data_number];
  for(int i=0;i<data_number;i++)Data[i]=new double[D_dim];



  int *expect = new int[data_number];

  for(int i=0;i<data_number;i++)
  {
     for(int d=0;d<D_dim;d++){
        Data[i][d]=data[i][d];
     }
     expect[i]=data[i][D_dim];
  }

int mmax = -99999;
            int mmin = 99999;
            for (int i = 0; i < data_number; i++)
            {
                if (expect[i] > mmax) mmax = expect[i];
                if (expect[i] < mmin) mmin = expect[i];
            }
            if (mmin == 1)
            {
                *categoryNum = mmax;
                for (int i = 0; i < data_number; i++) expect[i] = expect[i] - 1;

            }
            else
                *categoryNum = mmax + 1;



  bool *flag = new bool[data_number];

            for (int i = 0; i < data_number; i++)
                flag[i] = false;

            for(int i = 0; i < train_num; i++)
            {

                int k = rand()%data_number;

                while (flag[k])
                {
                    k = rand()%data_number;
                }

                flag[k] = true;

                trainExcept->push_back(expect[k]);//[i] = expect[k];

                vector<double> row;

                for (int j = 0; j < D_dim; j++)
                    row.push_back(Data[k][j]);//trainData[i][j] = Data[k][j];

                trainData->push_back(row);

            }



            // 資料集中剩下的當作測試集
            for (int i = 0, k = 0; i < data_number; i++)
            {

                if (flag[i])
                    continue; // 該筆已被當作訓練資料, 跳下一筆

                testExcept->push_back(expect[i]);//[k] = expect[i];

                vector<double> row2;

                for (int j = 0; j < D_dim; j++)
                    row2.push_back(Data[i][j]);//testData[k][j] = Data[i][j];
                testData->push_back(row2);

                k++;
            }

            for(int i=0;i<data_number;i++)delete Data[i];
            delete[] Data;

            delete[] flag;
}



void Jacobian::Discrete_Output( vector<double>* J_Output, int categoryNum)
{

  for (int i = 0; i < J_Output->size(); i++)
            {
                double mmin = DBL_MAX;//double.MaxValue;//;
                double index = 0;
                for (int j = 0; j < categoryNum; j++) {
                    double tmp = abs((*J_Output)[i] - j);
                    if (tmp < mmin) {
                        mmin = tmp;
                        index = j;
                    }
                }
                (*J_Output)[i] = index;
            }
}

double Jacobian::LMSTrain( vector <vector <double> > *J_hidden, vector<double>* Cluster_index, vector<double*>* Cluster_center, double** Data, double** Cluster_Center, int* trainExcept, int* Output, int data_number, int data_dimension, int Center_Number, int categoryNum,  char* richtextbox1, int MaxIteration)
{
            double correct = 0;
            double tmp_correct = 0;
            int iteration = 1;
            while (correct < 0.9 && iteration <= 20)
            {

                vector<double> J_Output;// = new vector<double>();
                vector<double> J_trainExcept;// = new vector<double>();
                //J_hidden = new vector<double**>();

                for (int j = 0; j < Cluster_index->size(); j++)
                {
                    vector<double*> K_data;// = new vector<double*>();
                    vector<double> K_desire;// = new vector<double>();
                    vector<double> K_trainExcept;// = new vector<double>();
                    for (int i = 0; i < data_number; i++)
                    {
                        if (Output[i] == j)
                        {
                            double* array = new double[data_dimension];
                            for (int d = 0; d < data_dimension; d++) array[d] = Data[i][d] - Cluster_Center[j][d];
                            K_data.push_back(array);
                            double delta_desire = (double)trainExcept[i] - (*Cluster_index)[j];//Output[i];
                            K_desire.push_back(delta_desire);
                            K_trainExcept.push_back(trainExcept[i]);
                            J_trainExcept.push_back(trainExcept[i]);
                        }
                    }

                    if (K_data.size() != 0)
                    {

                        double** A = new double*[K_data.size()];// data_dimension];
                        for(int i=0;i<K_data.size();i++)A[i]=new double[data_dimension];
                        double** B = new double*[K_data.size()];//, 1];

                        for(int i=0;i<K_data.size();i++)B[i]=new double[1];

                        //double[][] A= K_data.ToArray();
                        for (int i = 0; i < K_data.size(); i++) for (int d = 0; d < data_dimension; d++) A[i][d] = K_data[i][d];
                        for (int i = 0; i < K_data.size(); i++) for (int d = 0; d < 1; d++) B[i][d] = K_desire[i];
                        //double** Jweight= new double*[data_dimension];//1];
                        vector<double> Jweight;
                        for(int i=0;i<data_dimension;i++)Jweight.push_back(0);

                        int indx = 0;
                        while (indx < MaxIteration)//for(int indx = 0; indx < MaxIteration; indx++)
                        {
                            for (int i = 0; i < K_data.size(); i++)
                            {
                                double tmp=0;
                                for(int d=0;d<data_dimension;d++)
                                {
                                    tmp += Jweight[d]*K_data[i][d];//Jweight[d][0] *K_data[i][d];
                                }


                                double dis = 0;
                                for (int d = 0; d < data_dimension; d++)
                                {
                                    dis += K_data[i][d] * K_data[i][d];
                                }
                                //dis = Math.Sqrt(dis);
                                for (int d = 0; d < data_dimension; d++)
                                {
                                    Jweight[d] = Jweight[d] +  (K_desire[i] - tmp) * K_data[i][d]*(1/(5+dis));
                                  //Jweight[d][0] = Jweight[d][0] +  (K_desire[i] - tmp) * K_data[i][d]*(1/(5+dis));
                                }


                            }

                            indx++;
                        }

                        //var pinvA = A.PseudoInverse();
                        //var C = pinvA.Multiply(B);
                        //J_hidden.Add(C);
                        J_hidden->push_back(Jweight);
                        //var delta_d = A.Multiply(C);

                        mat matrix_A(K_data.size(),data_dimension);
                        mat matrix_Jweight(data_dimension,1);

                        for(int i=0;i<K_data.size();i++)for(int d=0;d<data_dimension;d++)matrix_A(i,d)=A[i][d];
                        for(int i=0;i<data_dimension;i++)for(int d=0;d<1;d++)matrix_Jweight(i,d)=Jweight[i];

                        mat delta_d = matrix_A*matrix_Jweight;//A.Multiply(Jweight);
                        for (int i = 0; i < K_data.size(); i++)
                        {
                            delta_d(i,0) += (*Cluster_index)[j];
                            J_Output.push_back(delta_d(i,0));
                        }

                        for(int i=0;i<K_data.size();i++)delete[] A[i];
                        delete[] A;


                        for(int i=0;i<K_data.size();i++)delete[] B[i];
                        delete[] B;

                    }
                    else
                    {
                        //double** C = new double*[data_dimension];//, 1];
                        //for(int i=0;i<data_dimension;i++)C[i]=new double[1];
                      vector<double> C;
                      J_hidden->push_back(C);
                    }

                }
                Discrete_Output(&J_Output, categoryNum);
                correct = 0;

                for (int i = 0; i < data_number; i++)
                {
                    if (J_Output[i] == J_trainExcept[i])
                    {
                       correct++;
                    }
                }
                correct = correct / (J_Output.size());
                //richtextbox1 += "\n";
char ss[256];
                sprintf(ss,"J number =%d\ntraining correct =%f\n",(int)Cluster_index->size(),correct);
                //std::stringstream ss;

printf("J number =%d\ntraining correct =%f\n",(int)Cluster_index->size(),correct);
                //richtextbox1 += "J number =" + Cluster_index.Count.ToString() + "\n";
                //richtextbox1 += "training correct =" + correct.ToString() + "\n";


                if (correct < 0.9 && iteration < 20)
                {
                    Cluster_index->clear();
                    Cluster_center->clear();

                    //Cluster_index = new vector<double>();
                    //Cluster_center = new vector<double[]>();
                    for(int i=0;i<Center_Number;i++)delete[] Cluster_Center[i];
                    delete[] Cluster_Center;
                    delete[] Output;

                    Center_Number += 1;

                    //Delete(Cluster_Center);

                    Cluster_Center = new double*[Center_Number];
                    for (int i = 0; i < Center_Number; i++) Cluster_Center[i] = new double[data_dimension];
                    Output = new int[data_number];
                    kmeans(Cluster_index, Cluster_center, Data, Output, Cluster_Center, trainExcept, data_number, data_dimension, Center_Number, categoryNum, MaxIteration);
                }


                iteration++;

            }


            return correct;

        }




void Jacobian::kmeans( vector<double>* Cluster_index, vector<double*>* Cluster_center,double **Data, int *Output, double** Cluster_Center, int* trainExpect, int data_number, int data_dimension, int Center_Number, int categoryNum,int MaxIteration) {



    //int MaxIteration=100;


    double MinDistance;
    float temp;
    int Close,Close_Center;
    srand(time(NULL));

    //Random rnd = new Random();

 for( int j = 0; j < Center_Number; j++)
 {
     int i=rand()%data_number;

    for( int d=0; d < data_dimension; d++)
    {
       Cluster_Center[j][d]=Data[i][d];
    }
 }

 for (int iter = 0; iter < MaxIteration; iter++)
 {

    for( int i = 0; i < data_number  ; i++)
    {
        Close_Center=-1;
        MinDistance = DBL_MAX;//MaxValue;//10000;
        for( int j = 0; j < Center_Number; j++)
        {
            double Distance=0;
            for( int d=0; d < data_dimension; d++)
               Distance+=(Data[i][d]-Cluster_Center[j][d])*(Data[i][d]-Cluster_Center[j][d]);
            Distance=sqrt(Distance);
            if(Distance==0)
                Close_Center=j;
            if(MinDistance>Distance)
            {
                MinDistance=Distance;
                Close_Center=j;
            }
        }

        if(Close_Center!=-1)
           Output[i]=Close_Center;
        else
           Output[i]=trainExpect[i];
    }

        // Caculate the New Cluster Center

    for( int j = 0; j < Center_Number; j++ )
    {
        for( int k = 0; k < (data_dimension)  ; k++)
        {
            Cluster_Center[j][k]=0;
        }

        temp=1;
        for( int i = 0; i < data_number  ; i++ )
        {
            if(Output[i]==j)
            {
               for( int k = 0; k < data_dimension  ; k++)
                  Cluster_Center[j][k]+=Data[i][k];
               temp++;
            }
        }
        for( int k = 0; k < (data_dimension)  ; k++)
            Cluster_Center[j][k]=Cluster_Center[j][k]/temp;

        if(temp==1)
        {
            int i = rand()%data_number;// rand() % data_number;
           for( int d=0; d < data_dimension; d++)
           {
              Cluster_Center[j][d]=Data[i][d];
           }
        }

    }
  }

    for( int i = 0; i < data_number  ; i++)
    {
        Close_Center=-1;
        MinDistance = DBL_MAX;//double.MaxValue;//;
        for( int j = 0; j < Center_Number; j++)
        {
            double Distance=0;
            for( int d=0; d < data_dimension; d++)
               Distance+=(Data[i][d]-Cluster_Center[j][d])*(Data[i][d]-Cluster_Center[j][d]);
            Distance=sqrt(Distance);
            if(Distance==0)
                Close_Center=j;
            if(MinDistance>Distance)
            {
                MinDistance=Distance;
Close_Center=j;
            }
        }

        //Output[i]=Close_Center;
        if (Close_Center != -1)
            Output[i] = Close_Center;
        else
            Output[i] = trainExpect[i];

    }

    //int* histogram = new int[categoryNum];
    //int* tmpOutput = new int[data_number];
    //for (int i = 0; i < data_number; i++) tmpOutput[i] = Output[i];
    for (int k = 0; k < Center_Number; k++)
    {
        //for (int i = 0; i < categoryNum; i++) histogram[i] = 0;
        //    for (int i = 0; i < data_number; i++)
        //    {
        //        if (tmpOutput[i] == k)
        //        {
        //            histogram[trainExpect[i]]++;
        //        }

        //    }
        //    int mmax = -int.MaxValue;//;
        //    int index=0;
        //    for (int i = 0; i < categoryNum; i++) {
        //        if (histogram[i] > mmax)
        //        {
        //            mmax = histogram[i];
        //            index = i;
        //        }
        //    }

            //for (int i = 0; i < data_number; i++)
            //{
            //    if (tmpOutput[i] == k)
            //    {
            //        Output[i]=index;
            //    }

            //}



            MinDistance = DBL_MAX;//double.MaxValue;
            int indexkk = 0;
            for (int i = 0; i < data_number; i++)
            {
                double Distance = 0;
                for (int d = 0; d < data_dimension; d++)
                    Distance += (Data[i][d] - Cluster_Center[k][d]) * (Data[i][d] - Cluster_Center[k][d]);
                Distance = sqrt(Distance);

                if (MinDistance > Distance)
                {
                    MinDistance = Distance;

                    indexkk = i;
                }
            }
            Cluster_index->push_back(trainExpect[indexkk]);
            //Cluster_index.Add(index);
            Cluster_center->push_back(Cluster_Center[k]);
    }



    }



int main(int argc,  char *argv[])
{
  printf("starting peogram\n");
//  char *ss ="579.txt";
  //scanf("file name: %s",ss);
  Jacobian JNN;// = new Jacobian();
  vector< vector <double> > trainData;// = new vector< vector <double> >();
  vector< vector <double> > testData;// =new vector< vector <double> >();
  vector<int> trainExpect;
  vector<int> testExpect;
  vector<double> Cluster_index;
  vector<double*> Cluster_center;
  vector< vector <double> > J_hidden;
  double **train_Data;
  double **test_Data;
  double **Cluster_Center;
  int *train_Expect;
  int *test_Expect;
  int *Output;
  int train_data_number=0;
  int test_data_number=0;
  int data_dimension=0;
  int categoryNum=0;
  int Center_Number = 1;
  int MaxIteration = 100;

  printf("starting reading file\n");

  JNN.ReadFile(argv[1],&trainData,&testData,&trainExpect,&testExpect,&train_data_number,&test_data_number,&data_dimension,&categoryNum);

  printf("end reading file\n");


  //train_data_number = train_data_number;//(int)trainData.size();
  //printf("....\n");
  //data_dimension = //trainData[0].size();
  //test_data_number= test_data_number;//testData.size();
  //printf(",,,,\n");

  printf("train num=%d\ntest num =%d\ndata_dimension=%d\n",train_data_number,test_data_number,data_dimension);

  train_Data = new double*[train_data_number];

  for(int i=0;i<train_data_number;i++)train_Data[i]=new double[data_dimension];

  test_Data = new double*[test_data_number];

  for(int i=0;i<test_data_number;i++)test_Data[i]=new double[data_dimension];

  Output = new int[train_data_number];

  Cluster_Center = new double*[Center_Number];
  for(int i=0;i<Center_Number;i++)Cluster_Center[i]=new double[data_dimension];

  train_Expect = new int[train_data_number];

  test_Expect = new int[test_data_number];


  for(int i=0;i<train_data_number;i++)for(int d=0;d<data_dimension;d++)train_Data[i][d]=trainData[i][d];

  for(int i=0;i<test_data_number;i++)for(int d=0;d<data_dimension;d++)test_Data[i][d]=testData[i][d];

  for(int i=0;i<train_data_number;i++)train_Expect[i]=trainExpect[i];

  for(int i=0;i<test_data_number;i++)test_Expect[i]=testExpect[i];


  JNN.kmeans(&Cluster_index,&Cluster_center,train_Data,Output, Cluster_Center,train_Expect,train_data_number,data_dimension,Center_Number,categoryNum,MaxIteration);

char richtextbox1[256]="output";//="output";

double train_correct=0;
double test_correct=0;
train_correct=JNN.LMSTrain( &J_hidden, &Cluster_index, &Cluster_center, train_Data, Cluster_Center, train_Expect, Output, train_data_number, data_dimension, Center_Number,  categoryNum,  richtextbox1, MaxIteration);

test_correct=JNN.Test(J_hidden, Cluster_index,Cluster_center, test_Data, test_Expect, test_data_number, data_dimension, categoryNum); printf("training correct rate=%f\n",train_correct);

printf("testing correct rate=%f\n",test_correct);

//printf(richtextbox1);
  printf("end program\n");

for(int i=0;i<train_data_number;i++)delete[] train_Data[i];
delete[] train_Data;
for(int i=0;i<test_data_number;i++)delete[] test_Data[i];
delete[] test_Data;
//for(int i=0;i<Center_Number;i++)delete[] Cluster_Center[i];
//delete[] Cluster_Center;
delete[] train_Expect;
delete[] test_Expect;

  return 0;
}



