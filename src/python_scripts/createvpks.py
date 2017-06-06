# This script creates VPK files for mod e.g. pak01_000.vpk, pak01_dir.vpk
# These VPK files are used to improve loading perfomance. 

# Put this file into modfolder e.g. reactivedropgit/reactivedrop 
# Change path to vpk.exe if needed 
# Run this script file using cmd: python createvpks.py

# User settings (don't use the \ character)
target_folders = [ "materials", "models", "particles", "scenes" ]
file_types = [ "vmt", "vtf", "mdl", "phy", "vtx", "vvd", "pcf", "vcd" ]
vpk_path = "E:/Games/Steam/SteamApps/common/alien swarm/bin/vpk.exe"
 
# Script begins
import os,subprocess
from os.path import join
response_path = join(os.getcwd(),"vpk_list.txt")
 
out = open(response_path,'w')
len_cd = len(os.getcwd()) + 1
 
for user_folder in target_folders:
	for root, dirs, files in os.walk(join(os.getcwd(),user_folder)):
		for file in files:
			if len(file_types) and file.rsplit(".")[-1] in file_types:
				out.write(os.path.join(root[len_cd:].replace("/","\\"),file) + "\n")
 
out.close()
 
subprocess.call([vpk_path, "-M", "a", "pak01", "@" + response_path])