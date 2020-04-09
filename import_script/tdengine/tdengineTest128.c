#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <taos.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
  char sql[256];
  char dataDir[256];
  int filesNum;
  int writeClients;
  int rowsPerRequest;
} ProArgs;

typedef struct {
  int64_t totalRows;
} StatisInfo;

typedef struct {
  pthread_t pid;
  int threadId;
  int sID;
  int eID;
} ThreadObj;

static StatisInfo statis;
static ProArgs arguments;

void parseArg(int argc, char *argv[]);

void writeData();

void readData();

int main(int argc, char *argv[]) {
  statis.totalRows = 0;
  parseArg(argc, argv);

  if (arguments.writeClients > 0) {
    writeData();
  } else {
    readData();
  }
}

void parseArg(int argc, char *argv[]) {
  strcpy(arguments.sql, "./sqlCmd.txt");
  strcpy(arguments.dataDir, "./testdata");
  arguments.filesNum = 2;
  arguments.writeClients = 0;
  arguments.rowsPerRequest = 100;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-sql") == 0) {
      if (i < argc - 1) {
        strcpy(arguments.sql, argv[++i]);
      }
      else {
        fprintf(stderr, "'-sql' requires a parameter, default:%s\n", arguments.sql);
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp(argv[i], "-dataDir") == 0) {
      if (i < argc - 1) {
        strcpy(arguments.dataDir, argv[++i]);
      }
      else {
        fprintf(stderr, "'-dataDir' requires a parameter, default:%s\n", arguments.dataDir);
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp(argv[i], "-numOfFiles") == 0) {
      if (i < argc - 1) {
        arguments.filesNum = atoi(argv[++i]);
      }
      else {
        fprintf(stderr, "'-numOfFiles' requires a parameter, default:%d\n", arguments.filesNum);
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp(argv[i], "-writeClients") == 0) {
      if (i < argc - 1) {
        arguments.writeClients = atoi(argv[++i]);
      }
      else {
        fprintf(stderr, "'-writeClients' requires a parameter, default:%d\n", arguments.writeClients);
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp(argv[i], "-rowsPerRequest") == 0) {
      if (i < argc - 1) {
        arguments.rowsPerRequest = atoi(argv[++i]);
      }
      else {
        fprintf(stderr, "'-rowsPerRequest' requires a parameter, default:%d\n", arguments.rowsPerRequest);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void taos_error(TAOS *con) {
  printf("TDengine error: %s\n", taos_errstr(con));
  taos_close(con);
  exit(1);
}

int64_t getTimeStampMs() {
  struct timeval systemTime;
  gettimeofday(&systemTime, NULL);
  return (int64_t)systemTime.tv_sec * 1000L + (int64_t)systemTime.tv_usec / 1000;
}

void writeDataImp(void *param) {
  ThreadObj *pThread = (ThreadObj *)param;
  printf("Thread %d, writing sID %d, eID %d\n", pThread->threadId, pThread->sID, pThread->eID);

  void *taos = taos_connect("127.0.0.1", "root", "taosdata", NULL, 0);
  if (taos == NULL)
    taos_error(taos);

  int code = taos_query(taos, "use db131");
  if (code != 0) {
    taos_error(taos);
  }

  char sql[65000];
  int sqlLen = 0;
  int lastMachineid = 0;
  int counter = 0;
  int totalRecords = 0;

  for (int j = pThread->sID; j <= pThread->eID; j++) {
    char fileName[256];
    sprintf(fileName, "%s/testdata%d.csv", arguments.dataDir, j);

    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
      printf("failed to open file %s\n", fileName);
      exit(1);
    }
    printf("open file %s success\n", fileName);

    char *line = NULL;
    size_t len = 0;
    while (!feof(fp)) {
      free(line);
      line = NULL;
      len = 0;

      getline(&line, &len, fp);
      if (line == NULL) break;

      if (strlen(line) < 10) continue;
      int machineid;
      char machinename[16];
      int machinegroup;
      int64_t timestamp;
      int temperature;
      float humidity;
      int c_int[63];
      double c_float[62];
      sscanf(line, "%d%s%d%lld%d%f%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d%lf%d", &machineid, machinename, &machinegroup, &timestamp, &temperature, &humidity, &c_int[0], &c_float[0], &c_int[1], &c_float[1], &c_int[2], &c_float[2], &c_int[3], &c_float[3], &c_int[4], &c_float[4], &c_int[5], &c_float[5], &c_int[6], &c_float[6], &c_int[7], &c_float[7], &c_int[8], &c_float[8], &c_int[9], &c_float[9], &c_int[10], &c_float[10], &c_int[11], &c_float[11], &c_int[12], &c_float[12], &c_int[13], &c_float[13], &c_int[14], &c_float[14], &c_int[15], &c_float[15], &c_int[16], &c_float[16], &c_int[17], &c_float[17], &c_int[18], &c_float[18], &c_int[19], &c_float[19], &c_int[20], &c_float[20], &c_int[21], &c_float[21], &c_int[22], &c_float[22], &c_int[23], &c_float[23], &c_int[24], &c_float[24], &c_int[25], &c_float[25], &c_int[26], &c_float[26], &c_int[27], &c_float[27], &c_int[28], &c_float[28], &c_int[29], &c_float[29], &c_int[30], &c_float[30], &c_int[31], &c_float[31], &c_int[32], &c_float[32], &c_int[33], &c_float[33], &c_int[34], &c_float[34], &c_int[35], &c_float[35], &c_int[36], &c_float[36], &c_int[37], &c_float[37], &c_int[38], &c_float[38], &c_int[39], &c_float[39], &c_int[40], &c_float[40], &c_int[41], &c_float[41], &c_int[42], &c_float[42], &c_int[43], &c_float[43], &c_int[44], &c_float[44], &c_int[45], &c_float[45], &c_int[46], &c_float[46], &c_int[47], &c_float[47], &c_int[48], &c_float[48], &c_int[49], &c_float[49], &c_int[50], &c_float[50], &c_int[51], &c_float[51], &c_int[52], &c_float[52], &c_int[53], &c_float[53], &c_int[54], &c_float[54], &c_int[55], &c_float[55], &c_int[56], &c_float[56], &c_int[57], &c_float[57], &c_int[58], &c_float[58], &c_int[59], &c_float[59], &c_int[60], &c_float[60], &c_int[61], &c_float[61], &c_int[62]);
      
  
      if (counter == 0) {
        sqlLen = sprintf(sql, "insert into");
      }

      if (lastMachineid != machineid) {
        lastMachineid = machineid;
        sqlLen += sprintf(sql + sqlLen, " dev%d using devices tags(%d,'%s',%d) values",
                          machineid, machineid, machinename, machinegroup);
      }

      sqlLen += sprintf(sql + sqlLen, "(%lld,%d,%f", timestamp, temperature, humidity);
      
      for(int i=0; i<62; ++i)
      {
        sqlLen += sprintf(sql + sqlLen,",%ld,%lf", c_int[i], &c_float[i]);
      }

      sqlLen += sprintf(sql + sqlLen, ",%ld)", c_int[62]);

      counter++;

      if (counter >= arguments.rowsPerRequest) {
        int code = taos_query(taos, sql);
        if (code != 0) {
          printf("thread:%d error:%d reason:%s\n", pThread->pid, code, taos_errstr(taos));
        }

        totalRecords += counter;
        counter = 0;
        lastMachineid = -1;
        sqlLen = 0;
      }
    }

    fclose(fp);
  }

  if (counter > 0) {
    int code = taos_query(taos, sql);
    if (code != 0) {
      printf("thread:%d error:%d reason:%s\n", pThread->pid, code, taos_errstr(taos));
    }

    totalRecords += counter;
  }

  __sync_fetch_and_add(&statis.totalRows, totalRecords);
}

void writeData() {
  printf("write data\n");
  printf("---- writeClients: %d\n", arguments.writeClients);
  printf("---- dataDir: %s\n", arguments.dataDir);
  printf("---- numOfFiles: %d\n", arguments.filesNum);
  printf("---- rowsPerRequest: %d\n", arguments.rowsPerRequest);

  taos_init();

  void *taos = taos_connect("127.0.0.1", "root", "taosdata", NULL, 0);
  if (taos == NULL)
    taos_error(taos);

  int code = taos_query(taos, "create database if not exists db131");
  if (code != 0) {
    taos_error(taos);
  }
  
  code = taos_query(taos, "create table if not exists db131.devices(ts timestamp, temperature int, humidity float, c1 int, c2 double, c3 int, c4 double, c5 int, c6 double, c7 int, c8 double, c9 int, c10 double, c11 int, c12 double, c13 int, c14 double, c15 int, c16 double, c17 int, c18 double, c19 int, c20 double, c21 int, c22 double, c23 int, c24 double, c25 int, c26 double, c27 int, c28 double, c29 int, c30 double, c31 int, c32 double, c33 int, c34 double, c35 int, c36 double, c37 int, c38 double, c39 int, c40 double, c41 int, c42 double, c43 int, c44 double, c45 int, c46 double, c47 int, c48 double, c49 int, c50 double, c51 int, c52 double, c53 int, c54 double, c55 int, c56 double, c57 int, c58 double, c59 int, c60 double, c61 int, c62 double, c63 int, c64 double, c65 int, c66 double, c67 int, c68 double, c69 int, c70 double, c71 int, c72 double, c73 int, c74 double, c75 int, c76 double, c77 int, c78 double, c79 int, c80 double, c81 int, c82 double, c83 int, c84 double, c85 int, c86 double, c87 int, c88 double, c89 int, c90 double, c91 int, c92 double, c93 int, c94 double, c95 int, c96 double, c97 int, c98 double, c99 int, c100 double, c101 int, c102 double, c103 int, c104 double, c105 int, c106 double, c107 int, c108 double, c109 int, c110 double, c111 int, c112 double, c113 int, c114 double, c115 int, c116 double, c117 int, c118 double, c119 int, c120 double, c121 int, c122 double, c123 int, c124 double, c125 int)"
    "tags(devid int, devname binary(16), devgroup int)");
  if (code != 0) {
    taos_error(taos);
  }

  int64_t st = getTimeStampMs();

  int a = arguments.filesNum / arguments.writeClients;
  int b = arguments.filesNum % arguments.writeClients;
  int last = 0;

  ThreadObj *threads = calloc((size_t)arguments.writeClients, sizeof(ThreadObj));
  for (int i = 0; i < arguments.writeClients; ++i) {
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
    pthread_create(&pthread->pid, &thattr, (void *(*)(void *))writeDataImp, pthread);
  }

  for (int i = 0; i < arguments.writeClients; i++) {
    pthread_join(threads[i].pid, NULL);
  }

  int64_t elapsed = getTimeStampMs() - st;
  float seconds = (float)elapsed / 1000;
  float rs = (float)statis.totalRows / seconds;

  printf("---- Spent %f seconds to insert %ld records, speed: %f Rows/Second\n", seconds, statis.totalRows, rs);
}

void readData() {
  printf("read data\n");
  printf("---- sql: %s\n", arguments.sql);

  void *taos = taos_connect("127.0.0.1", "root", "taosdata", NULL, 0);
  if (taos == NULL)
    taos_error(taos);

  FILE *fp = fopen(arguments.sql, "r");
  if (fp == NULL) {
    printf("failed to open file %s\n", arguments.sql);
    exit(1);
  }
  printf("open file %s success\n", arguments.sql);

  char *line = NULL;
  size_t len = 0;
  while (!feof(fp)) {
    free(line);
    line = NULL;
    len = 0;

    getline(&line, &len, fp);
    if (line == NULL) break;

    if (strlen(line) < 10) continue;

    int64_t st = getTimeStampMs();

    int code = taos_query(taos, line);
    if (code != 0) {
      taos_error(taos);
    }

    void *result = taos_use_result(taos);
    if (result == NULL) {
      printf("failed to get result, reason:%s\n", taos_errstr(taos));
      exit(1);
    }

    TAOS_ROW row;
    int rows = 0;
    //int num_fields = taos_field_count(taos);
    //TAOS_FIELD *fields = taos_fetch_fields(result);
    while ((row = taos_fetch_row(result))) {
      rows++;
      //char        temp[256];
      //taos_print_row(temp, row, fields, num_fields);
      //printf("%s\n", temp);
    }

    taos_free_result(result);

    int64_t elapsed = getTimeStampMs() - st;
    float seconds = (float)elapsed / 1000;
    printf("---- Spent %f seconds to query: %s", seconds, line);
  }

  fclose(fp);
}
