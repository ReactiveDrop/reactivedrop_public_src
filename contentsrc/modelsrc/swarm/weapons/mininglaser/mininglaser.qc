$modelname "weapons/MiningLaser/MiningLaser.mdl"

$body "default" "mininglaser_model.smd"

$bodygroup "magazine"
{
	studio "mininglaser_battery.smd"
	blank
}
$bodygroup "grip"
{
	studio "mininglaser_grip.smd"
	blank
}
$bodygroup "maw"
{
	studio "mininglaser_maw.smd"
	studio "mininglaser_maw2.smd"
	studio "mininglaser_maw3.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/MiningLaser"

$texturegroup "skinfamilies"
{
	{ "mininglaser" }
	{ "TeslaCannon" }
}

$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0
$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0

$sequence "BindPose" "mininglaser_model.smd"

$collisionmodel "mininglaser_physics.smd"
