import re
import os
import sys
import getopt

tvDalPath = os.getenv('TV_DAL_PATH', os.path.join(os.path.sep, "home","buildengineer", "test_visualizer", "scripts"))
sys.path.append(tvDalPath)

try:
  from data_access_layer import insert_data
except ImportError:
  inser_data_imported = False
else:
  inser_data_imported = True

def getThroughputAverageFromLog(pattern, logs):
  regex = re.compile(pattern, re.DOTALL)
  matches=regex.finditer(logs)
  
  total=0
  numberOfMatches=0
  for match in matches:
    print match.group(0).split("\n")[0]+" "+match.group(1)+" bits/s"
    total=total+int(match.group(1))
    numberOfMatches=numberOfMatches+1
  
  averageThroughput=total/numberOfMatches
  print "Average: "+str(averageThroughput)+" bits/s"
  return averageThroughput

def getAverageSuccessRateFromLog(pattern, logs):
  regex = re.compile(pattern, re.DOTALL)
  matches=regex.finditer(logs)
  
  totalRecieved=0
  totalSent=0
  for match in matches:
    print match.group(0).split("\n")[0]+" "+match.group(1)+"/"+match.group(2)+ " messages"

    totalRecieved=totalRecieved+int(match.group(1))
    totalSent=totalSent+int(match.group(2))
  
  averageSuccessRate=totalRecieved/totalSent*100
  print "Success Rate: "+str(averageSuccessRate)+"%"
  return averageSuccessRate

def printHelp():
    print 'parseAndUploadThroughputData.py -i <inputfile> -n'
    print '   -n, --noupload -  Do not upload data. Good for local result examination.'
    print '   -i, --ifile    - Use <inputfile> as alternate input file, instead of the default (dmp.log).'

def main(argv):

  inputfile = "dmp.log"
  isUpload = True

  try:
    opts, args = getopt.getopt(argv,"hi:n",["ifile="])
  except getopt.GetoptError:
    printHelp()
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      printHelp()
      sys.exit()
    elif opt in ("-i", "--ifile"):
       inputfile = arg
    elif opt in ("-n", "--noupload"):
       isUpload = False

  print ''
  if not inser_data_imported:
    print 'Cannot load inser_data module.'
    isUpload = False
  if not isUpload:
    print 'Upload is disabled!'

  print '\nUsed log file: ', inputfile, '\n'

  with open (inputfile, "r") as myfile:
    data=myfile.read()
  
  print "Test Summary: \n"
  print "No BLE Activity:"
  print "---------------------------------"
  averageThroughput=getThroughputAverageFromLog(r'no BLE activity - dmp\d -> dmp\d:.*?Throughput:\s(\d*)\sbits/s', data)
  if isUpload:
    insert_data(value=averageThroughput,
              data_type='Throughput',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["No BLE Activity"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))
  print ""
  averageSuccessRate=getAverageSuccessRateFromLog(r'no BLE activity - dmp\d -> dmp\d:.*?Success messages: (\d*) out of (\d*)', data)
  if isUpload:
    insert_data(value=averageSuccessRate,
              data_type='Success Rate',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["No BLE Activity"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))

  print ""
  print "BLE Advertising:"
  print "---------------------------------"
  averageThroughput=getThroughputAverageFromLog(r'BLE advertising \(dmp\d\) - dmp\d -> dmp\d:.*?Throughput:\s(\d*)\sbits/s', data)
  if isUpload:
    insert_data(value=averageThroughput,
              data_type='Throughput',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Advertising"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))
  print ""
  averageSuccessRate=getAverageSuccessRateFromLog(r'BLE advertising \(dmp\d\) - dmp\d -> dmp\d:.*?Success messages: (\d*) out of (\d*)', data)
  if isUpload:
    insert_data(value=averageSuccessRate,
              data_type='Success Rate',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Advertising"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))

  print ""
  print "BLE Connection:"
  print "---------------------------------"
  averageThroughput=getThroughputAverageFromLog(r'BLE connection - dmp\d -> dmp\d:.*?Throughput:\s(\d*)\sbits/s', data)
  if isUpload:
    insert_data(value=averageThroughput,
              data_type='Throughput',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Connection"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))
  print ""
  averageSuccessRate=getAverageSuccessRateFromLog(r'BLE connection - dmp\d -> dmp\d:.*?Success messages: (\d*) out of (\d*)', data)
  if isUpload:
    insert_data(value=averageSuccessRate,
              data_type='Success Rate',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Connection"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))

  print ""
  print "BLE Connection and BLE Advertising:"
  print "---------------------------------"
  averageThroughput=getThroughputAverageFromLog(r'BLE connection and BLE advertisement - dmp\d -> dmp\d:.*?Throughput:\s(\d*)\sbits/s', data)
  if isUpload:
    insert_data(value=averageThroughput,
              data_type='Throughput',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Connection and BLE Advertising"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))
  print ""
  averageSuccessRate=getAverageSuccessRateFromLog(r'BLE connection and BLE advertisement - dmp\d -> dmp\d:.*?Success messages: (\d*) out of (\d*)', data)
  if isUpload:
    insert_data(value=averageSuccessRate,
              data_type='Success Rate',
              commit=os.environ.get("GIT_COMMIT"),
              metadata=["BLE Connection and BLE Advertising"],
              branch_name=os.environ.get("BRANCH_NAME"),
              build_url=os.environ.get("BUILD_URL"))

if __name__ == "__main__":
  main(sys.argv[1:])