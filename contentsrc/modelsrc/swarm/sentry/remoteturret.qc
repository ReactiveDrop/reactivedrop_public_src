$modelname "swarm\SentryGun\remoteturret.mdl"

$body Body "remoteturret.smd"

$surfaceprop "weapon"

$contents "solid"

$illumposition 1.848 -11.036 10.814

$cdmaterials "swarm\models\SentryGun\"

$attachment "muzzle" "sentgun" -41.92 8 0 rotate 0 -180 90
$attachment "camera" "sentgun" 0 16 11 rotate 0 -180 90

$cbox 0 0 0 0 0 0

$bbox -20.801 -7.604 -7.762 42.873 11.3 29.391

$definebone "sentgun" "" 0.215927 -0.780876 72.859703 0 89.999982 89.999982 0 0 0 0 0 0
$definebone "sentgun_body" "sentgun" -0.117969 7.667984 -0.016851 0 0 0 0 0 0 0 0 0
$definebone "sentgun_drum" "sentgun_body" -3.117743 -2.612167 6.993868 0 0 0 0 0 0 0 0 0
$definebone "sentgun_Muzzle" "sentgun_body" -34.210953 0.000023 0.000029 0 0 0 0 0 0 0 0 0
$definebone "sentgun_antenna" "sentgun_body" 13.485376 2.405785 -0.000011 0 0 0 0 0 0 0 0 0
$definebone "sentgun_antenna_tip" "sentgun_antenna" 0.000002 17.617126 0 0 0 0 0 0 0 0 0 0

$sequence "Bindpose" {
	"anims\Bindpose.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "fire" {
	"anims\fire.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "idle" {
	"anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "idle_off" {
	"anims\idle_off.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "idle_off_reversed" {
	"anims\idle_off_reversed.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "turn_on" {
	"anims\turn_on.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "turn_on_reversed" {
	"anims\turn_on_reversed.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "turn_off" {
	"anims\turn_off.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "turn_off_reversed" {
	"anims\turn_off_reversed.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "remoteturret_physics.smd"
{
	$mass 15.60251
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
}
