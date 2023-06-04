$modelname weapons/accessories/strange_device.mdl
$surfaceprop weapon

$cdmaterials models/weapons/accessories/strange_device

$body device strange_device.dmx
$sequence BindPose anims/BindPose.dmx

$opaque

$lod 2 {
	replacemodel strange_device.dmx strange_device_lod1.dmx
}
