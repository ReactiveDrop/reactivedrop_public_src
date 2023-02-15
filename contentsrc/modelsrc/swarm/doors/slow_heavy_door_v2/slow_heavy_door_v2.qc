$modelname "props/doors/slow_heavy_door/slow_heavy_door_v2.mdl"

$body "default" "slow_heavy_door_v2_model.smd"

$surfaceprop "solidmetal"
$contents "solid"
$opaque

$cdmaterials "models/props/doors/v2"

$texturegroup "skinfamilies"
{
	{ "heavy_slow_door_v2_01" }
	{ "heavy_slow_door_v2_02" }
	{ "heavy_slow_door_v2_03" }
	{ "heavy_slow_door_v2_04" }
	{ "heavy_slow_door_v2_05" }
	{ "heavy_slow_door_v2_06" }
	{ "heavy_slow_door_v2_07" }
	{ "heavy_slow_door_v2_08" }
	{ "heavy_slow_door_v2_09" }
}

$bbox -16 -80 0 0 80 160

$sequence "BindPose" "slow_heavy_door_v2_model.smd"

$sequence "open" {
	"anims/open.smd"
	{ event AE_CL_PLAYSOUND 0 "asw.slow.heavydoor.open" }
	fps 24
}

$sequence "open_silent" {
	"anims/open.smd"
	fps 24
}

$sequence "close" {
	"anims/close.smd"
	{ event AE_CL_PLAYSOUND 0 "ASW_Door.LandongBayDoor01Open" }
	fps 120
	compress 5
}

$sequence "close_silent" {
	"anims/close.smd"
	fps 120
	compress 5
}

$collisionjoints "slow_heavy_door_v2_physics.smd"
{
	$mass 10000
}
