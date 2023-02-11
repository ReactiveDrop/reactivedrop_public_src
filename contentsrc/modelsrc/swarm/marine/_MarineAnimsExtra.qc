//==============================================================================
// Marine Animations (Extra; demand-loaded)
//==============================================================================





//==============================================================================
// the model + basics
//==============================================================================
$modelname swarm/marine/marine_anims_extra.mdl
$addsearchdir dmx/
$addsearchdir dmx/animation/
$animblocksize 32

//==============================================================================
// includes
//==============================================================================
$include "_DefineBones.qci"
$include "_HierarchyChainsWeights.qci"
$include "_BoneMerges.qci"

$sequence extra_test "Marine.dmx" {
	hidden
}
