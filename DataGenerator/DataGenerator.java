import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.channels.spi.SelectorProvider;
import java.text.DecimalFormat;
import java.util.Random;

public class DataGenerator {
    /*
     * to simulate the change action of humidity The valid range of humidity is
     * [0, 100]
     */
    public static class ValueGen {
        int center;
        int range;
        Random rand;

        public ValueGen(int center, int range) {
            this.center = center;
            this.range = range;

            this.rand = new Random();
        }

        double next() {
            double v = this.rand.nextGaussian();
            if (v < -3) {
                v = -3;
            }

            if (v > 3) {
                v = 3;
            }

            return (this.range / 3.00) * v + center;
        }
    }

    // data scale
    private static int timestep = 1000; // sample time interval in milliseconds

    private static long dataStartTime = 1563249700000L;
    
    private static String tagPrefix = "dev_";

    // MachineNum RowsPerMachine MachinesInOneFile
    public static void main(String args[]) {
        int numOfDevice = 10000;
        int numOfFiles = 100;
        int rowsPerDevice = 10000;
        int bigData = 0;
        String dolphindb_directory = "~/";
        String tdengine_directory = "~/";


        for (int i = 0; i < args.length; i++) {
            if (args[i].equalsIgnoreCase("-numOfDevices")) {
                if (i < args.length - 1) {
                    numOfDevice = Integer.parseInt(args[++i]);
                } else {
                    System.out.println("'-numOfDevices' requires a parameter, default is 10000");
                }
            } else if (args[i].equalsIgnoreCase("-numOfFiles")) {
                if (i < args.length - 1) {
                    numOfFiles = Integer.parseInt(args[++i]);
                } else {
                    System.out.println("'-numOfFiles' requires a parameter, default is 100");
                }
            } else if (args[i].equalsIgnoreCase("-rowsPerDevice")) {
                if (i < args.length - 1) {
                    rowsPerDevice = Integer.parseInt(args[++i]);
                } else {
                    System.out.println("'-rowsPerDevice' requires a parameter, default is 10000");
                }
            } else if (args[i].equalsIgnoreCase("-bigData")) {
                if (i < args.length - 1) {
                    bigData = Integer.parseInt(args[++i]);
                } else {
                    System.out.println("'-bigData' requires a parameter, default is false");
                }
            } else if (args[i].equalsIgnoreCase("-dolphindb_dataDir")) {
                if (i < args.length - 1) {
                    dolphindb_directory = args[++i];
                } else {
                    System.out.println("'-dolphindb_dataDir' requires a parameter, default is ~/dolphindb_testdata");
                }
            }else if (args[i].equalsIgnoreCase("-tdengine_dataDir")) {
                if (i < args.length - 1) {
                    tdengine_directory = args[++i];
                } else {
                    System.out.println("'-tdengine_dataDir' requires a parameter, default is ~/tdengine_testdata");
                }
            }
        }

        System.out.println("parameters");
        System.out.printf("----dolphindb_dataDir:%s\n", dolphindb_directory);
        System.out.printf("----tdengine_dataDir:%s\n", tdengine_directory);
        System.out.printf("----numOfFiles:%d\n", numOfFiles);
        System.out.printf("----bigData:%d\n", bigData);
        System.out.printf("----numOfDevice:%d\n", numOfDevice);
        System.out.printf("----rowsPerDevice:%d\n", rowsPerDevice);
      

        int numOfDevPerFile = numOfDevice / numOfFiles;
        long ts = dataStartTime;

        // deviceId, time stamp, humid(int), temp(double), tagString(dev_deviceid)
        int humidityDistRadius = 35;
        int tempDistRadius = 17;
        
        int deviceId = 0;
        
        for (int i = 0; i < numOfFiles; ++i) { // prepare the data file
            dataStartTime = ts;
            deviceId=i*numOfDevPerFile;
           // generate file name
            String dolphindb_path = dolphindb_directory;
            String tdengine_path = tdengine_directory;
            try {
                dolphindb_path += "/testdata" + String.valueOf(i) + ".csv";
                tdengine_path += "/testdata" + String.valueOf(i) + ".csv";
                getDataInOneFile(deviceId, dolphindb_path, tdengine_path, rowsPerDevice, numOfDevPerFile, bigData, humidityDistRadius, tempDistRadius);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    private static void getDataInOneFile(int deviceId, String dolphindb_path, String tdengine_path, int rowsPerDevice, int num, int bigData, int humidityDistRadius, int tempDistRadius) throws IOException {
    	int devId = deviceId;
    	
    	DecimalFormat df = new DecimalFormat("0.0000");
        long startTime = dataStartTime;

        FileWriter fw_dolphindb = new FileWriter(new File(dolphindb_path));
        BufferedWriter bw_dolphindb = new BufferedWriter(fw_dolphindb);

        FileWriter fw_tdengine = new FileWriter(new File(tdengine_path));
        BufferedWriter bw_tdengine = new BufferedWriter(fw_tdengine);

        for (int j = 0; j < rowsPerDevice; ++j){
        	
        	deviceId = devId;
        	Random rand = new Random();
            double centralVal = Math.abs(rand.nextInt(100));
            if (centralVal < humidityDistRadius) {
                centralVal = humidityDistRadius;
            }

            if (centralVal + humidityDistRadius > 100) {
                centralVal = 100 - humidityDistRadius;
            }

            DataGenerator.ValueGen humidityDataGen = new DataGenerator.ValueGen((int) centralVal, humidityDistRadius);

            centralVal = Math.abs(rand.nextInt(22));
            DataGenerator.ValueGen tempDataGen = new DataGenerator.ValueGen((int) centralVal, tempDistRadius);

        	for (int i = 0; i < num; ++i) {
                deviceId += 1;

                int humidity = (int) humidityDataGen.next();
                double temp = tempDataGen.next();
                int deviceGroup = deviceId % 100;

                StringBuffer sb_dolphindb = new StringBuffer();
                StringBuffer sb_tdengine = new StringBuffer();  

                sb_dolphindb.append(deviceId).append(",").append(tagPrefix).append(deviceId).append(",").append(deviceGroup)
                       .append(",").append(dataStartTime).append(",").append(humidity).append(",")
                       .append(df.format(temp));

                sb_tdengine.append(deviceId).append(" ").append(tagPrefix).append(deviceId).append(" ").append(deviceGroup)
                       .append(" ").append(dataStartTime).append(" ").append(humidity).append(" ")
                       .append(df.format(temp));

                if(bigData == 1)
                {
                    for(int tt=0; tt<62; ++tt)
                    {
                        sb_dolphindb.append(",").append(humidity).append(",").append(temp);
                        sb_tdengine.append(" ").append(humidity).append(" ").append(temp);
                    }
                    sb_dolphindb.append(",").append(humidity);
                    sb_tdengine.append(" ").append(humidity);
                }
                
                bw_dolphindb.write(sb_dolphindb.toString());
                bw_dolphindb.write("\n");
        
                bw_tdengine.write(sb_tdengine.toString());
                bw_tdengine.write("\n");

        	}
        	dataStartTime += timestep;
        	
        }
        
        bw_dolphindb.close();
        fw_dolphindb.close();

        bw_tdengine.close();
        fw_tdengine.close();
        
        System.out.printf("file:%s generated\n", dolphindb_path);
        System.out.printf("file:%s generated\n", tdengine_path);
        
    }
}
