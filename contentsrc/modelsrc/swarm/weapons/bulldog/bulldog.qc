$modelname "weapons/deagle/deagle.mdl"

$bodygroup "magazine"
{
	studio "bulldog_reference.smd"
	studio "bulldog_reference_unloaded.smd"
}
$bodygroup "laser"
{
	studio "bulldog_laser.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$mostlyopaque

$cdmaterials "swarm/models/ASDecorationSkins"
$cdmaterials "models/weapons/deagle"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "bulldog_reference.smd"

$collisionmodel "bulldog_physics.smd"
