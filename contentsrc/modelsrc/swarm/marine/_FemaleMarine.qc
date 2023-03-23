//==============================================================================
// For legacy reasons, this model is used for the entire first row of marines.
// The model is named female, the materials are named both male and female, and it's all a big mess that doesn't mean anything.
//==============================================================================





//==============================================================================
// the model + basics
//==============================================================================
$modelname swarm/marine/femalemarine.mdl
$cdmaterials models/player/male
$cdmaterials models/player/male/back_units
$addsearchdir dmx/
$addsearchdir dmx/animation/
$eyeposition 0 0 70
$body body "Marine.dmx"

$bodygroup back_units
{
	blank
	studio "dmx/officer_back_unit.dmx"
	studio "dmx/medic_back_unit.dmx"
	studio "dmx/special_weapons_back_unit.dmx"
	studio "dmx/tech_back_unit.dmx"
}

$bodygroup helmets
{
	studio "dmx/marine_helmet.dmx"
	blank
}

$surfaceprop "flesh"
$bbox -13 -13 0  13 13 72

$texturegroup skinfamilies
{
	{ "medic_body" "medic_helmet" }
	{ "officer_body_female" "officer_helmet_female" }
	{ "medic_body_female" "medic_helmet_female" }
	{ "special_weapons_body_female" "special_weapons_helmet_female" }
	{ "tech_body_female" "tech_helmet_female" }
}

$renamematerial "officer_back_unit" "officer_back_unit_female"
$renamematerial "medic_back_unit" "medic_back_unit_female"
$renamematerial "special_weapons_back_unit" "special_weapons_back_unit_female"
$renamematerial "tech_back_unit" "tech_back_unit_female"

//==============================================================================
// includes
//==============================================================================
$include "_DefineBones.qci"
$include "_HierarchyChainsWeights.qci"
$include "_BoneMerges.qci"

$includemodel swarm/marine/marine_anims.mdl
$includemodel swarm/marine/marine_anims_extra.mdl

$include "_Attachments.qci"
$include "_Hitbox.qci"
$include "_Ragdoll.qci"
$include "_JiggleBones.qci"
