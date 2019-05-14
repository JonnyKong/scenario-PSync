# -*- coding: utf-8 -*-
"""
    Parse ndnSim output logs.
    @Author jonnykong@cs.ucla.edu
    @Date   2019-04-07
"""

import os, sys
import numpy as np

class DataInfo:
    def __init__(self, birth):
        self.GenerationTime = birth
        self.LastTime = birth
        self.Owner = 1
        self.Available = False


class SyncDuration:
    
    def __init__(self, filename):
        self._filename = filename
        self._node_num = 20
        self._availability_threshold = int(self._node_num * 0.95)
        # self._availability_threshold = 2 

        self._data_sync_duration = []
        self._data_store = {}
        self._state_sync_duration = []
        self._state_store = {}
        self._n_sync_interest = 0
        self._n_sync_reply = 0
        self._n_data_interest = 0
        self._n_data_reply = 0

    def run(self):
        self._getSyncDuration()
        self._printSyncDuration()
    
    def _getSyncDuration(self):
        with open(self._filename, "r") as f:
            for line in f.readlines():
                if line.find("microseconds") == -1:
                    continue
                elif line.find("Store New Data") != -1:
                    self._processDataSyncDuration(line)
                elif line.find("Update New Seq") != -1:
                    self._processStateSyncDuration(line)
                elif line.find("Send Sync Interest") != -1:
                    self._processSyncInterest(line)
                elif line.find("Send Sync Reply") != -1:
                    self._processSyncReply(line)
                elif line.find("Send Data Interest") != -1:
                    self._processDataInterest(line)
                elif line.find("Send Data Reply") != -1:
                    self._processDataReply(line)
                # else:
                #     print(line)
                #     raise AssertionError()  # For debug

    def _processDataSyncDuration(self, line):
        elements = line.strip().split(' ')
        time = elements[0]
        data_name = elements[-1]
        if data_name not in self._data_store:
            self._data_store[data_name] = DataInfo(int(time))
        else:
            self._data_store[data_name].Owner += 1
            self._data_store[data_name].LastTime = int(time)
        data_info = self._data_store[data_name]
        if data_info.Owner > self._node_num:
            raise AssertionError()
        elif not data_info.Available and data_info.Owner >= self._availability_threshold:
            self._data_store[data_name].Available = True
            cur_sync_duration = data_info.LastTime - data_info.GenerationTime
            cur_sync_duration = float(cur_sync_duration) / 1000000.0
            self._data_sync_duration.append(cur_sync_duration)

    def _processStateSyncDuration(self, line):
        elements = line.strip().split(' ')
        time = elements[0]
        data_name = elements[-1]
        if data_name not in self._state_store:
            self._state_store[data_name] = DataInfo(int(time))
        else:
            self._state_store[data_name].Owner += 1
            self._state_store[data_name].LastTime = int(time)
        data_info = self._state_store[data_name]
        if data_info.Owner > self._node_num:
            print("DATA INFO: ")
            print(line)
            raise AssertionError()
        elif not data_info.Available and data_info.Owner >= self._availability_threshold:
            self._state_store[data_name].Available = True
            cur_sync_duration = data_info.LastTime - data_info.GenerationTime
            cur_sync_duration = float(cur_sync_duration) / 1000000.0
            self._state_sync_duration.append(cur_sync_duration)
    
    def _processSyncInterest(self, line):
        self._n_sync_interest += 1

    def _processSyncReply(self, line):
        self._n_sync_reply += 1

    def _processDataInterest(self, line):
        self._n_data_interest += 1

    def _processDataReply(self, line):
        self._n_data_reply += 1

    def _printSyncDuration(self):
        data_availability = float(len(self._data_sync_duration)) / float(len(self._data_store))
        print("data availability = " + str(data_availability))
        
        data_sync_delay = np.mean(self._data_sync_duration)
        print("data sync delay = " + str(data_sync_delay))
        
        state_sync_delay = np.mean(self._state_sync_duration)
        print("state sync delay = " + str(state_sync_delay))

        print("out notify interest = " + str(self._n_sync_interest))
        print("out ack = " + str(self._n_sync_reply))
        print("out data interest = " + str(self._n_data_interest))
        print("out data = " + str(self._n_data_reply))

        n_data = len(self._data_store)
        print("Number of app data produced = " + str(n_data))

        # print("Number of owners for each data:")
        # for data, datainfo in self._data_store.items():
        #     print(str(datainfo.Owner), end=' ')
        # print("")

        # print(self._data_sync_duration)


if __name__ == "__main__":
    sd = SyncDuration(sys.argv[1])
    sd.run()
