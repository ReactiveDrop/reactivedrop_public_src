$modelname "weapons/MarksmanRifle/MarksmanRifle.mdl"

$body "default" "marksmanrifle_model.smd"

$bodygroup "magazine"
{
	studio "marksmanrifle_mag_new.smd"
	blank
}
$bodygroup "grip"
{
	studio "marksmanrifle_grip.smd"
	studio "marksmanrifle_grenadelauncher_grip.smd"
}
$bodygroup "scope"
{
	studio "marksmanrifle_scope.smd"
	blank
}
$bodygroup "laser"
{
	studio "marksmanrifle_laser.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/MarksmanRifle"
$cdmaterials "swarm/models/ASDecorationSkins"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "marksmanrifle_model.smd"

$collisionmodel "marksmanrifle_physics.smd"
