$modelname "props/doors/door001a_256/door001a_256.mdl"

$bodygroup "default"
{
	studio "door001a_256_doublesided.smd"
	studio "door001a_256_damaged_front.smd"
	studio "door001a_256_damaged_heavy_front.smd"
	studio "door001a_256_damaged_back.smd"
	studio "door001a_256_damaged_heavy_back.smd"
}

$surfaceprop "metal"
$contents "solid"
$opaque

$cdmaterials "models/props/doors"

$bbox -8 -64 2 8 64 144

$sequence "BindPose" "door001a_256_doublesided.smd"

$sequence "door_rear_fall" {
	"anims/fall_backward.smd"
	{ event AE_START_SCRIPTED_EFFECT 5 "" }
	{ event AE_START_SCRIPTED_EFFECT 10 "" }
	{ event AE_START_SCRIPTED_EFFECT 15 "" }
	{ event AE_CL_PLAYSOUND 0 "ASW_Door.MeleeDown256" }
	fps 24
}

$sequence "door_front_fall" {
	"anims/fall_forward.smd"
	{ event AE_START_SCRIPTED_EFFECT 5 "" }
	{ event AE_START_SCRIPTED_EFFECT 10 "" }
	{ event AE_START_SCRIPTED_EFFECT 15 "" }
	{ event AE_CL_PLAYSOUND 0 "ASW_Door.MeleeDown256" }
	fps 24
}

$collisionmodel "door001a_256_physics.smd"
{
	$mass 1600
}

$keyvalues
{
	asw_door
	{
		hit_sound "ASW_Door.MeleeSheet"
		dent_sound "ASW_Door.MeleeStrain"
	}
}
