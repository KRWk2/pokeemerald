RusturfTunnel_B1F_MapScripts::
	.byte 0

RusturfTunnel_B1F_TotemBattle::
	lock
	checktrainerflag TRAINER_ROXANNE_1
	goto_if 0, RusturfTunnel_B1F_TotemBattle_1
	msgbox RusturfTunnel_B1F_TotemBattle_Text_1
	closemessage
	settotemboost 1, 1, 0
	setwildbattle SPECIES_RAYQUAZA_MEGA, 100, ITEM_LIFE_ORB
	dowildbattle
	end

RusturfTunnel_B1F_TotemBattle_1:
	msgbox RusturfTunnel_B1F_TotemBattle_Text_0
	end
	return


RusturfTunnel_B1F_TotemBattle_Text_0:
	.string "go beat roxanne$"

RusturfTunnel_B1F_TotemBattle_Text_1:
	.string "roxanne calls$"
