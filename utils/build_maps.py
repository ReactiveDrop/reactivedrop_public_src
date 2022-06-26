# this script recompiles maps, 
# and for each map: 
# - creates stringtabledictionary: stringtabledictionary
# - creates sound manifest: snd_writemanifest
# - builds cubemaps: buildcubemaps
# 
import os,subprocess,shutil,sys
from subprocess import call

os.environ["RD_STEAM"] = "D:/Program Files/Steam/steamapps/common/Alien Swarm Reactive Drop"
os.environ["RD_DEV"] = "C:/Users/dmitriy/work/reactivedropgit"

# list of maps to compile
# each item is a list of: ["MapFileName", "radius_override", "buildcubemaps? 0 or 1 string"]
VMFs = [
##		["asi-jac1-landingbay_01", "3000", "0"],
##		["asi-jac1-landingbay_02", "2500", "0"],
##		["asi-jac1-landingbay_pract", "3000", "0"],
##		["asi-jac2-deima", "2500", "0"],
##		["asi-jac3-rydberg", "2500", "0"],
##		["asi-jac4-residential", "2500", "0"],
##		["asi-jac6-sewerjunction", "2500", "0"],
##		["asi-jac7-timorstation", "2500", "0"],
##		["rd-bio1operationx5", "750", "0"],
##		["rd-bio2invisiblethreat", "750", "0"],
##		["rd-bio3biogenlabs", "750", "0"],
##		["dm_deima", "2500", "0"],
##		["dm_desert", "1500", "0"],
##		["dm_residential", "1500", "0"],
##		["dm_testlab", "750", "0"],
##		["nest01cave", "2500", "0"],
##		["nest02bunker", "2500", "0"],
##		["nest03rapture", "2500", "0"],
##		["rd-area9800lz", "1500", "0"],
##		["rd-area9800pp1", "1500", "0"],
##		["rd-area9800pp2", "750", "0"],
##		["rd-area9800wl", "1500", "0"],
##		["rd-bonus_mission1", "2500", "0"],
##		["rd-bonus_mission2", "2500", "0"],
##		["rd-bonus_mission3", "750", "0"],
##		["rd-bonus_mission4", "3000", "0"],
##		["rd-bonus_mission5", "750", "1"],
##		["rd-bonus_mission6", "1500", "1"],
##		["rd-bonus_mission7", "1500", "0"],
##		["rd-dc1_omega_city", "1500", "0"],
##		["rd-dc2_breaking_an_entry", "750", "0"],
##		["rd-dc3_search_and_rescue", "750", "0"],
##		["rd-lan1_bridge", "2500", "0"],
##		["rd-lan2_sewer", "1500", "0"],
##		["rd-lan3_maintenance", "750", "0"],
##		["rd-lan4_vent", "1500", "0"],
##		["rd-lan5_complex", "750", "0"],
##		["rd-ocs1storagefacility", "750", "0"],
##		["rd-ocs2landingbay7", "750", "0"],
##		["rd-ocs3uscmedusa", "750", "0"],
##		["rd-ori1niosarefinary", "1500", "0"],
##		["rd-ori2firstanomaly", "1500", "0"],
##		["rd-par1unexpected_encounter", "1500", "1"],
##		["rd-par2hostile_places", "750", "1"],
##		["rd-par3close_contact", "750", "1"],
##		["rd-par4high_tension", "1500", "1"],
##		["rd-par5crucial_point", "1500", "1"],
##		["rd-res1forestentrance", "750", "0"],
##		["rd-res2research7", "1500", "0"],
##		["rd-res3miningcamp", "1500", "0"],
##		["rd-res4mines", "1500", "0"],
##		["rd-tft1desertoutpost", "3000", "0"],
##		["rd-tft2abandonedmaintenance", "1500", "0"],
##		["rd-tft3spaceport", "2000", "0"],
##		["rd-til1midnightport", "1500", "0"],
##		["rd-til2roadtodawn", "750", "0"],
##		["rd-til3arcticinfiltration", "2500", "1"],
##		["rd-til4area9800", "750", "0"],
##		["rd-til5coldcatwalks", "1500", "0"],
##		["rd-til6yanaurusmine", "750", "0"],
##		["rd-til7factory", "750", "0"],
##		["rd-til8comcenter", "1500", "0"],
##		["rd-til9syntekhospital", "750", "0"],
##		["rd-reduction1", "2500", "0"],
##		["rd-reduction2", "2500", "0"],
##		["rd-reduction3", "2500", "0"],
##		["rd-reduction4", "2500", "0"],
##		["rd-reduction5", "2500", "0"],
##		["rd-reduction6", "2500", "0"],
##		["rd-nh01_logisticsarea", "2500", "1"],
##		["rd-nh02_platformxvii", "2500", "1"],
##		["rd-nh03_groundworklabs", "2500", "1"],
		]

game = os.environ["RD_STEAM"]
vbsp = game + "/bin/vbsp.exe"
vvis = game + "/bin/vvis.exe"
vrad = game + "/bin/vrad.exe"
gameexe = game + "/reactivedrop.exe"

# mapsrc is a folder where VMF are
mapsrc = os.environ["RD_DEV"] + "/contentsrc/mapsrc"
# for Alien Swarm replace this to "C:/Program Files (x86)/Steam/steamapps/common/Alien Swarm/swarm"
# it's where your GameInfo.txt file resides
moddir = os.environ["RD_DEV"] + "/reactivedrop"
# compiled maps will be copied here
mapdir = moddir + "/maps"
# script will create a cfg file here to automate buildcubemaps etc
cfgdir = moddir + "/cfg"
build_all_maps_cfg = cfgdir + "/build_all_maps.cfg"

if not os.path.exists(mapdir):
	os.makedirs(mapdir, mode=0o777, exist_ok=True)

# write first part of the cfg file
with open(build_all_maps_cfg, "w") as myfile:
	myfile.write("alias wait1 \"echo 10;wait 100;echo 9;wait 100;echo 8;wait 100;echo 7;wait 100;echo 6;wait 100;echo 5;wait 100;echo 4;wait 100;echo 3;wait 100;echo 2;wait 100;echo 1;wait 100;disconnect;buildnextmap\"\n")
	myfile.write("alias buildcubemaps1 \"wait 1000;buildcubemaps 2;wait1\"\n")
	myfile.write("alias buildmap0 \"wait 1000;stringtabledictionary;wait 100;snd_writemanifest;wait 100;asw_restart_mission;wait1\"\n")
	myfile.write("alias buildmap1 \"wait 1000;stringtabledictionary;wait 100;snd_writemanifest;wait 100;buildcubemaps1\"\n")
	myfile.write("alias build_exit \"exit\"\n")

# compile maps and copy to the 'maps' folder
for i, mapinfo in enumerate(VMFs):
	call([vbsp, "-alldetail", "-game", moddir, mapsrc + "/" + mapinfo[0]])
	call([vvis, "-radius_override", mapinfo[1], "-game", moddir, mapsrc + "/" + mapinfo[0]])
	call([vrad, "-low", "-final", "-textureshadows", "-StaticPropLighting", "-StaticPropPolys", "-game", moddir, mapsrc + "/" + mapinfo[0]])
	#call([vrad, "-low", "-threads", "4", "-fast", "-game", moddir, mapsrc + "/" + mapinfo[0]])
	try:
		shutil.copy2(mapsrc + "/" + mapinfo[0] + ".bsp", mapdir + "/" + mapinfo[0] + ".bsp")
	except:
		print("Couldn't copy map %s" % mapinfo[0])
		raise
	# write code into cfg file which requires map names
	nextmap = None
	if i == len(VMFs) - 1:
		nextmap = "exit"
	else:
		nextmap = "map" + str(i + 1)
	with open(build_all_maps_cfg, "a") as myfile:
		myfile.write("alias build_%s \"map %s;alias buildnextmap build_%s;buildmap%s\n" % ("map" + str(i),mapinfo[0],nextmap,mapinfo[2]))
		
# footer of the cfg file
with open(build_all_maps_cfg, "a") as myfile:
	myfile.write("sv_cheats 1\ndeveloper 1\nsv_allow_wait_command 1\n")
	myfile.write("build_map0\n")

# run the game and execute the cfg file
# game will load each map and execute these commands using 
# aliases and wait command: stringtabledictionary;snd_writemanifest;buildcubemaps;
call([gameexe, "-novid", "-windowed", "-w", "1280", "-h", "720", "-game", moddir, "+exec build_all_maps"])
