from subprocess import check_call
import sys, os

vtex_exe = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\bin\vtex.exe"
game_dir = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\reactivedrop"
out_dir = "../reactivedrop/materials/vgui/letters"

for letter in sys.argv[1:]:
	label = ""
	smooth = ["-blur", "0x5", "-threshold", "50%", "-blur", "0x1"]
	if len(letter) == 1:
		if ord(letter) < 97 or ord(letter) > 122:
			raise Exception("invalid single letter (must be ASCII and lowercase; otherwise use a codepoint)")
		label = "label:" + letter.upper()
	else:
		if len(letter) != 4 or letter.lower() != letter:
			raise Exception("invalid codepoint (should be 4 lowercase hex digits)")
		codepoint = bytes.fromhex(letter).decode("utf-16le")
		label = "label:" + codepoint
		if ord(codepoint) >= 0x2E80:
			# more complicated letters get messed up by rounding; don't do it for CJK
			smooth = []

	check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", "Noto-Sans-CJK-SC-Black", "-pointsize", "224", "-gravity", "south", "-size", "256x256", label] + smooth + ["xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + ".tga"])
	check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", "Noto-Sans-CJK-SC-Black", "-pointsize", "224", "-gravity", "south", "-size", "256x256", label] + smooth + ["(", "-clone", "0", "-blur", "0x7.5", "-clone", "0,0,0,0,0", "-composite", ")", "-composite", "xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + "_glow.tga"])
	check_call([vtex_exe, "-game", game_dir, "-outdir", out_dir, "-nopause", "letter_" + letter + ".tga"])
	check_call([vtex_exe, "-game", game_dir, "-outdir", out_dir, "-nopause", "letter_" + letter + "_glow.tga"])
	os.unlink("letter_" + letter + ".tga")
	os.unlink("letter_" + letter + "_glow.tga")
	with open(out_dir + "/letter_" + letter + ".vmt", "w") as f:
		f.write("""UnlitGeneric
{
$basetexture "vgui\\letters\\letter_%s"
$translucent 1
$distancealpha 1
$softedges 1
$edgesoftnessstart .6
$edgesoftnessend .2
$scaleedgesoftnessbasedonscreenres 1
$vertexcolor 1
$vertexalpha 1
$no_fullbright 1
$ignorez 1
}""" % letter)
	with open(out_dir + "/letter_" + letter + "_glow.vmt", "w") as f:
		f.write("""UnlitGeneric
{
$basetexture "vgui\\letters\\letter_%s_glow"
$translucent 1
$vertexcolor 1
$vertexalpha 1
$ignorez 1
}""" % letter)
