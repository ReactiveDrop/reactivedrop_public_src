$modelname "swarm/doors/swarm_singledoor_flipped.mdl"

$bodygroup "default"
{
	studio "swarm_singledoor_ref_v2.smd"
	studio "singledoor_dmg_front.smd"
	studio "singledoor_dmg_heavy_front.smd"
	studio "singledoor_dmg_back.smd"
	studio "singledoor_dmg_heavy_back.smd"
}

$surfaceprop "metal"
$contents "solid"
$opaque

$cdmaterials "swarm/models/doors"

$texturegroup "skinfamilies"
{
	{ "SingleDoorSkin1" }
	{ "SingleDoorSkinReinforced" }
	{ "SingleDoorUndestroyable" }
}

$bbox -10.145 -72.591 -8.295 10.085 78.627 149.606

$sequence "BindPose" "swarm_singledoor_ref_v2.smd"

$sequence "door_rear_fall" {
	"anims/fall_backward.smd"
	{ event AE_START_SCRIPTED_EFFECT 5 "" }
	{ event AE_START_SCRIPTED_EFFECT 10 "" }
	{ event AE_START_SCRIPTED_EFFECT 15 "" }
	{ event AE_CL_PLAYSOUND 0 "ASW_Door.MeleeDownSingle" }
	fps 24
}

$sequence "door_front_fall" {
	"anims/fall_forward.smd"
	{ event AE_START_SCRIPTED_EFFECT 5 "" }
	{ event AE_START_SCRIPTED_EFFECT 10 "" }
	{ event AE_START_SCRIPTED_EFFECT 15 "" }
	{ event AE_CL_PLAYSOUND 0 "ASW_Door.MeleeDownSingle" }
	fps 24
}

$collisionmodel "swarm_singledoor_physics.smd"
{
	$mass 300
}
