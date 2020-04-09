#include "DolphinDB.h"
#include "Util.h"
#include "pthread.h"
#include "sys/time.h"
#include "SysIO.h"
#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
using namespace dolphindb;
using namespace std;

typedef struct {
  pthread_t pid;
  int threadId;
  int sID;
  int eID;
} ThreadObj;

//server config
string HOST="192.168.1.112";
long PORT=8848;

//parameters
static int numOfFiles=100; //文件数
static int writeClients=10; //线程数
static int rowsPerRequest=100; //每次请求写入条数
static int totalRows=numOfFiles*1000000; //每个文件总行数
static string dataDir="/media/hj/aa/data3/dolphindb_testdata"; //测试数据路径
static string queryDir="/media/hj/aa/DolphinDB_TDengine/query_script/dolphindb_query.txt"; //查询脚本路径

// defined functions
void createDatabase(DBConnection conn);
static void writeData(void *param);
void readData(DBConnection conn, string queryScript);
int64_t getTimeStampMs();

int main(int argc, char *argv[]){

    Socket::ENABLE_TCP_NODELAY=true;

    //Connect to a DolphinDB server
    DBConnection conn;
    conn.initialize();
    bool ret = conn.connect(HOST, PORT, "admin", "123456");
    if(!ret){
        cout<<"Failed to connect to the server"<<endl;
        return 0;
    }

    //query
    if(argv[1]!=NULL)
    {
        readData(conn,queryDir);
        return 0;    
    }
    int64_t st = getTimeStampMs();

    //create
    createDatabase(conn);

    //create threads
    int a = numOfFiles / writeClients;
    int b = numOfFiles % writeClients;
    int last = 0;

    ThreadObj *threads =  (ThreadObj *)calloc((size_t)writeClients, sizeof(ThreadObj));
    for(int i=0; i<writeClients; ++i)
    { 
        ThreadObj *pthread = threads + i;
        pthread_attr_t thattr;
        pthread->threadId = i + 1;
        pthread->sID = last;
        if (i < b) {
            pthread->eID = last + a;
        } else {
          pthread->eID = last + a - 1;
        }
        last = pthread->eID + 1;
        pthread_attr_init(&thattr);
        pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_JOINABLE);
        pthread_create(&pthread->pid, &thattr, (void *(*)(void *))writeData, pthread);
    } 

    //retrieve resources
    for (int i = 0; i < writeClients; i++)
    {
        pthread_join(threads[i].pid, NULL);
    }

    //end time
    int64_t elapsed = getTimeStampMs() - st;
    float seconds = (float)elapsed / 1000;
    float rs = (float)totalRows / seconds;
    
    //print time elapsed
    cout<<"Spent "<<seconds<<" seconds to insert "<<totalRows<<" records, speed: "<<rs<<" Rows/Second"<<endl;
    
    //get result
    string script_result="exec count(*) from loadTable('dfs://devDB',`dev)";
    cout<<"exec count(*) from loadTable('dfs://devDB',`dev):"<<endl;
    conn.connect(HOST, PORT, "admin", "123456");
    ConstantSP result = conn.run(script_result);
    cout<<result->getString()<<endl;
    return 0;
}

void createDatabase(DBConnection conn)
{
    string script_schema="cols=[ 'devId', 'devName', 'devGroup', 'time', 'temperature', 'humidity'] \n types=[INT, SYMBOL, INT, LONG, INT, DOUBLE] \n schema = table(500:0, cols, types)\n";
    string script_createDatabase="dbDir = 'dfs://devDB'\n if(existsDatabase(dbDir))\n dropDatabase(dbDir)\n range_schema=0..100 * 100 + 1\n db = database(dbDir, RANGE, range_schema)\n dev = db.createPartitionedTable(schema, `dev , `devId)\n";
    conn.run(script_schema + script_createDatabase);
    cout<<"database created"<<endl;
}

static void writeData(void *param){
    
    try{
        ThreadObj *pThread = (ThreadObj *)param;
        cout<<pThread->threadId<<" sID: "<<pThread->sID<<" eID: "<<pThread->eID<<endl;
        string data;
        DBConnection conn; //Connect to a DolphinDB server
        conn.initialize();
        bool ret = conn.connect(HOST, PORT, "admin", "123456");
        if(!ret){
            cout<<"Failed to connect to the server"<<endl;
            exit(-1);
        }
        for(int i=pThread->sID;i<=pThread->eID;++i)
        {
            data=dataDir+"/testdata"+std::to_string(i)+".csv";
            
            //create table
            TableSP t = conn.run("loadText('"+data+"')");
            VectorSP range = Util::createPair(DT_INDEX);
            
            INDEX start = 0;
            vector<ConstantSP> args;
            while (start < t->size())
            {
                range->setIndex(0, start);
                range->setIndex(1, start + rowsPerRequest);
                args.clear();
                args.push_back(t->get(range));
                conn.run("tableInsert{loadTable('dfs://devDB',`dev)}", args); 
                start += rowsPerRequest;
            }
            t.clear();
            cout<<"File "<<data<<" imported"<<endl;
        }
    }catch(exception& e){
        cout<<"Error occurred: "<<e.what()<<endl;
    }
}

void readData(DBConnection conn, string queryScript)
{
    ifstream inFile(queryScript,ios::in);
	if(!inFile.is_open())
	{
	    cout<<"Failed to open "<<queryScript<<endl;
		exit(-1);
	}
	string lineStr;
    vector<string> strArray;
    while(getline(inFile,lineStr))
    {
        strArray.push_back(lineStr);
    }
	inFile.close();
    
    conn.run("dev=loadTable('dfs://devDB', `dev)"); //load table
    for(uint8_t i=0; i<strArray.size(); ++i)
    {
        conn.run("clearAllCache()"); //clear cache
        int64_t st = getTimeStampMs();//start time
        // ConstantSP result = conn.run(strArray.at(i));//query
        conn.run(strArray.at(i));//query
        int64_t elapsed = getTimeStampMs() - st;//end time
        float seconds = (float)elapsed / 1000;
        // cout<<result->getString()<<endl;
        cout<<"Spent "<<seconds<<" seconds to query: '"<<strArray.at(i)<<"'"<<endl;//print time elapsed
    }
}

int64_t getTimeStampMs() {
  struct timeval systemTime;
  gettimeofday(&systemTime, NULL);
  return (int64_t)systemTime.tv_sec * 1000L + (int64_t)systemTime.tv_usec / 1000;
}
