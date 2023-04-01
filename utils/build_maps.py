# this script recompiles maps, 
# and for each map: 
# - creates stringtabledictionary: stringtabledictionary
# - builds cubemaps: buildcubemaps
# 
import os,subprocess,shutil,sys
from subprocess import call

os.environ["RD_STEAM"] = "D:/Program Files/Steam/steamapps/common/Alien Swarm Reactive Drop"
os.environ["RD_DEV"] = "C:/Users/dmitriy/work/reactivedropgit"

class MapInfo:
	def __init__(self, name, radius_override="2500", buildcubemaps="1", vbsp=None, vvis=None, vrad=None):
		self.name = name.lower()
		self.buildcubemaps = buildcubemaps
		self.vbsp = vbsp or ["-alldetail"]
		self.vvis = vvis or ["-radius_override", radius_override]
		self.vrad = vrad or ["-final", "-textureshadows", "-StaticPropLighting", "-StaticPropPolys"]

vrad_notextureshadows = ["-final", "-StaticPropLighting", "-StaticPropPolys"]

# list of maps to compile
# each item is a MapInfo ("MapFileName", "radius_override", "buildcubemaps? 0 or 1 string")
VMFs = [
##		MapInfo("asi-jac1-landingbay_01", "3000", "0"),
##		MapInfo("asi-jac1-landingbay_02", "2500", "0"),
##		MapInfo("asi-jac1-landingbay_pract", "3000", "0"),
##		MapInfo("asi-jac2-deima", "2500", "0"),
##		MapInfo("asi-jac3-rydberg", "2500", "0"),
##		MapInfo("asi-jac4-residential", "2500", "0"),
##		MapInfo("asi-jac6-sewerjunction", "2500", "0"),
##		MapInfo("asi-jac7-timorstation", "2500", "0"),
##		MapInfo("example_map_1", "1500", "1"),
##		MapInfo("example_map_2", "1500", "1"),
##		MapInfo("example_map_3", "1500", "1"),
##		MapInfo("rd-bio1operationx5", "750", "0"),
##		MapInfo("rd-bio2invisiblethreat", "750", "0"),
##		MapInfo("rd-bio3biogenlabs", "750", "0"),
##		MapInfo("dm_deima", "2500", "0"),
##		MapInfo("dm_desert", "1500", "0"),
##		MapInfo("dm_residential", "1500", "0"),
##		MapInfo("dm_testlab", "750", "0"),
##		MapInfo("nest01cave", "2500", "0"),
##		MapInfo("nest02bunker", "2500", "0"),
##		MapInfo("nest03rapture", "2500", "0"),
##		MapInfo("rd-area9800lz", "1500", "0"),
##		MapInfo("rd-area9800pp1", "1500", "0"),
##		MapInfo("rd-area9800pp2", "750", "0"),
##		MapInfo("rd-area9800wl", "1500", "0"),
##		MapInfo("rd-bonus_mission1", "2500", "0"),
##		MapInfo("rd-bonus_mission2", "2500", "0"),
##		MapInfo("rd-bonus_mission3", "750", "0"),
##		MapInfo("rd-bonus_mission4", "3000", "0"),
##		MapInfo("rd-bonus_mission5", "750", "1"),
##		MapInfo("rd-bonus_mission6", "1500", "1"),
##		MapInfo("rd-bonus_mission7", "1500", "0"),
##		MapInfo("rd-bonus10_sewrev", "2500", "0"),
##		MapInfo("rd-bonus12_rydrev", "2500", "0"),
##		MapInfo("rd-bonus14_cargrev", "2500", "0"),
##		MapInfo("rd-bonus15_landrev", "3000", "0"),
##		MapInfo("rd-dc1_omega_city", "1500", "0"),
##		MapInfo("rd-dc2_breaking_an_entry", "750", "0"),
##		MapInfo("rd-dc3_search_and_rescue", "750", "0"),
##		MapInfo("rd-lan1_bridge", "2000", "0"),
##		MapInfo("rd-lan2_sewer", "1500", "0"),
##		MapInfo("rd-lan3_maintenance", "750", "0"),
##		MapInfo("rd-lan4_vent", "1500", "0"),
##		MapInfo("rd-lan5_complex", "750", "0"),
##		MapInfo("rd-ocs1storagefacility", "750", "0"),
##		MapInfo("rd-ocs2landingbay7", "750", "0"),
##		MapInfo("rd-ocs3uscmedusa", "750", "0"),
##		MapInfo("rd-ori1niosarefinary", "1500", "0"),
##		MapInfo("rd-ori2firstanomaly", "1500", "0"),
##		MapInfo("rd-par1unexpected_encounter", "1500", "1"),
##		MapInfo("rd-par2hostile_places", "750", "1"),
##		MapInfo("rd-par3close_contact", "750", "1"),
##		MapInfo("rd-par4high_tension", "1500", "1"),
##		MapInfo("rd-par5crucial_point", "1500", "1"),
##		MapInfo("rd-res1forestentrance", "750", "0"),
##		MapInfo("rd-res2research7", "1500", "0"),
##		MapInfo("rd-res3miningcamp", "1500", "0"),
##		MapInfo("rd-res4mines", "1500", "0"),
##		MapInfo("rd-tft1desertoutpost", "3000", "0"),
##		MapInfo("rd-tft2abandonedmaintenance", "1500", "0"),
##		MapInfo("rd-tft3spaceport", "2000", "0"),
##		MapInfo("rd-til1midnightport", "1500", "0"),
##		MapInfo("rd-til2roadtodawn", "750", "0"),
##		MapInfo("rd-til3arcticinfiltration", "2500", "1"),
##		MapInfo("rd-til4area9800", "750", "0"),
##		MapInfo("rd-til5coldcatwalks", "1500", "0"),
##		MapInfo("rd-til6yanaurusmine", "750", "0"),
##		MapInfo("rd-til7factory", "750", "0"),
##		MapInfo("rd-til8comcenter", "1500", "0"),
##		MapInfo("rd-til9syntekhospital", "750", "0"),
##		MapInfo("rd-reduction1", "2500", "0"),
##		MapInfo("rd-reduction2", "2500", "0"),
##		MapInfo("rd-reduction3", "2500", "0"),
##		MapInfo("rd-reduction4", "2500", "0"),
##		MapInfo("rd-reduction5", "2500", "0"),
##		MapInfo("rd-reduction6", "2500", "0"),
##		MapInfo("rd-nh01_logisticsarea", "2500", "1"),
##		MapInfo("rd-nh02_platformxvii", "2500", "1"),
##		MapInfo("rd-nh03_groundworklabs", "2500", "1"),
##		MapInfo("rd-acc1_infodep", "1200", "0"),
##		MapInfo("rd-acc2_powerhood", "900", "0"),
##		MapInfo("rd-acc3_rescenter", "1500", "0", vrad=vrad_notextureshadows),
##		MapInfo("rd-acc4_confacility", "900", "1", vrad=vrad_notextureshadows),
##		MapInfo("rd-acc5_j5connector", "1200", "1", vrad=vrad_notextureshadows),
##		MapInfo("rd-acc6_labruins", "1500", "1", vrad=vrad_notextureshadows),
##		MapInfo("rd-acc_complex", "1500", "0", vrad=vrad_notextureshadows),
##		MapInfo("rd-ada_sector_a9", "1152", "1"),
##		MapInfo("rd-ada_nexus_subnode", "1152", "1"),
##		MapInfo("rd-ada_neon_carnage", "768", "1"),
##		MapInfo("rd-ada_fuel_junction", "1512", "1"),
##		MapInfo("rd-ada_dark_path", "1152", "1"),
##		MapInfo("rd-ada_forbidden_outpost", "2048", "1"),
##		MapInfo("rd-ada_new_beginning", "1152", "1"),
##		MapInfo("rd-ada_anomaly", "1152", "1"),
##		MapInfo("rd-ht-marine_academy", "5000", "0"),
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
	myfile.write("alias buildmap0 \"wait 1000;stringtabledictionary;wait 100;asw_restart_mission;wait1\"\n")
	myfile.write("alias buildmap1 \"wait 1000;stringtabledictionary;wait 100;buildcubemaps1\"\n")
	myfile.write("alias build_exit \"exit\"\n")

# compile maps and copy to the 'maps' folder
for i, mapinfo in enumerate(VMFs):
	name = mapsrc + "/" + mapinfo.name
	call([vbsp] + mapinfo.vbsp + ["-game", moddir, name])
	call([vvis] + mapinfo.vvis + ["-game", moddir, name])
	call([vrad, "-low"] + mapinfo.vrad + ["-game", moddir, name])
	try:
		shutil.copy2(mapsrc + "/" + mapinfo.name + ".bsp", mapdir + "/" + mapinfo.name + ".bsp")
	except:
		print("Couldn't copy map %s" % mapinfo.name)
		raise
	# write code into cfg file which requires map names
	nextmap = None
	if i == len(VMFs) - 1:
		nextmap = "exit"
	else:
		nextmap = "map" + str(i + 1)
	with open(build_all_maps_cfg, "a") as myfile:
		myfile.write("alias build_%s \"map %s;alias buildnextmap build_%s;buildmap%s\n" % ("map" + str(i),mapinfo.name,nextmap,mapinfo.buildcubemaps))
		
# footer of the cfg file
with open(build_all_maps_cfg, "a") as myfile:
	myfile.write("sv_cheats 1\ndeveloper 1\nsv_allow_wait_command 1\nasw_instant_restart 0\n")
	if len(VMFs) == 0:
		myfile.write("exit\n")
	else:
		myfile.write("build_map0\n")

# run the game and execute the cfg file
# game will load each map and execute these commands using 
# aliases and wait command: stringtabledictionary;buildcubemaps;
call([gameexe, "-novid", "-windowed", "-w", "1280", "-h", "720", "-skiploadingworkshopaddons", "-game", moddir, "+exec build_all_maps"])
