#include "rom_database.h"
#include "internal.h"
#include "types.h"

// this can be optimised by splitting into 2 tables
// one with just the crc32, the other with the rest of
// the data at the SAME index.
// this way, looping through the crc32 table will be much
// faster, then once the entry is found, index the second
// table to get the rest of the data.
// however, it's important that the 2 tables match, and can
// be a pain to maintain otherwise...

// NOTE: not EVERY game in this table has been tested.
// all sg1000 games have.
// sega mappers are untested as they all behave the same way
// and knowing the amount of ram a game uses isn't important (it seems)
static const struct RomEntry ENTRIES[] =
{
// SMS
/* -----------------------------------------------------*/
    // 20-em-1 (B) [!].sms
	{ .crc = 0xF0F35C22, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// ALF (U) [!].sms
	{ .crc = 0x82038AD4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ace of Aces (E) [!].sms
	{ .crc = 0x887D9F6B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Action Fighter (J) [!].sms
	{ .crc = 0xD91B340D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Action Fighter (UE) [!].sms
	{ .crc = 0x3658F3E0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Addams Family, The (E) [!].sms
	{ .crc = 0x72420F38, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aerial Assault (UE) [!].sms
	{ .crc = 0xECF491CF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// After Burner (UE) [!].sms
	{ .crc = 0x978C2927, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// After Burner (UE) [b1].sms
	{ .crc = 0xFC79B585, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Air Rescue (E) [!].sms
	{ .crc = 0x8B43D21D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Air Rescue (E) [b1].sms
	{ .crc = 0x8B4CA9E4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Air Rescue (E) [b1][T+Bra_Emutrans].sms
	{ .crc = 0x7337ABB7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Air Rescue (E) [T+Bra_Emutrans].sms
	{ .crc = 0x7338D04E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aladdin (E) [!].sms
	{ .crc = 0xC8718D40, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aladdin (E) [T+Bra_Imp4444].sms
	{ .crc = 0xF69C331A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aladdin (E) [T+Fre_GenerationIX].sms
	{ .crc = 0x615D8D35, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aladdin (E) [T-Bra70%_lmp4444].sms
	{ .crc = 0x5CB24C41, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd - BMX Trial (J) [!].sms
	{ .crc = 0xF9DBB533, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd - The Lost Stars (UE) [!].sms
	{ .crc = 0xC13896D5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd - The Lost Stars (UE) [b1].sms
	{ .crc = 0x2210F389, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in High Tech World (UE) [!].sms
	{ .crc = 0x013C0A94, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in High Tech World (UE) [T+Bra_GamesBR].sms
	{ .crc = 0xDD6277FB, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in High Tech World (UE) [T+Ger1.00_Star-trans].sms
	{ .crc = 0x29220201, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Anmitsu Hime (J) [!].sms
	{ .crc = 0xFFF63B0B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (B) (V1.1) [p1][!].sms
	{ .crc = 0x7545D7C2, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (B) (V1.1) [p1][b1].sms
	{ .crc = 0x9F10E7F8, .rom = 0xFC5C, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (B) (V1.1) [p1][o1].sms
	{ .crc = 0x2C7278AD, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (J) [!].sms
	{ .crc = 0x08C9EC91, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (J) [T+Fre_Terminus].sms
	{ .crc = 0x9C99ED7B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (U) (V1.0) [T+Ita1.0_Anthrax].sms
	{ .crc = 0x4213FA16, .rom = 0x3F1B6, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (U) (V1.0).sms
	{ .crc = 0x17A40E29, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [!].sms
	{ .crc = 0xAED9AAC4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [o1].sms
	{ .crc = 0x99BA12CD, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [o2].sms
	{ .crc = 0xF8144914, .rom = 0x40040, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T+Bra_CBT].sms
	{ .crc = 0x8938B5C8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T+Fre].sms
	{ .crc = 0x944BED08, .rom = 0x2FC5A, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T+Fre][a1].sms
	{ .crc = 0xB33817AE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T+Ita1.0].sms
	{ .crc = 0x5BCD77ED, .rom = 0x3F1B6, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T-Bra_CBT].sms
	{ .crc = 0xD7D01766, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Miracle World (UE) (V1.1) [T-Bra_CBT][a1].sms
	{ .crc = 0x61273F35, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Shinobi World (UE) [!].sms
	{ .crc = 0xD2417ED7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Shinobi World (UE) [b1].sms
	{ .crc = 0x2CD7DEC7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Shinobi World (UE) [b2].sms
	{ .crc = 0x99B590D5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alex Kidd in Shinobi World (UE) [T+Fre_Terminus].sms
	{ .crc = 0x587E41D5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alien 3 (E) [!].sms
	{ .crc = 0xB618B144, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alien Storm (E) [!].sms
	{ .crc = 0x62A2846F, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alien Syndrome (J) [!].sms
	{ .crc = 0x4CC11DF9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Alien Syndrome (UE) [!].sms
	{ .crc = 0x412D9A6B, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Altered Beast (UE) [!].sms
	{ .crc = 0xA6308D64, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Andre Agassi Tennis (E) [!].sms
	{ .crc = 0xF499034D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Andre Agassi Tennis (E) [T+Bra][p1][!].sms
	{ .crc = 0x009F8630, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Arcade Smash Hits (E) [!].sms
	{ .crc = 0xE4163163, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Argos no Juujiken (J) [!].sms
	{ .crc = 0xBAE75805, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Argos no Juujiken (J) [a1][!].sms
	{ .crc = 0x32DA4B0D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ariel - The Little Mermaid (B).sms
	{ .crc = 0xF4B3A7BD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Assault City - Light Phaser Version (E) [!].sms
	{ .crc = 0x861B6E79, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Assault City - Pad Version (E) [!].sms
	{ .crc = 0x0BD8DA96, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Asterix and the Great Rescue (E) [!].sms
	{ .crc = 0xF9B7D26B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// As Aventuras da TV Colosso (B).sms
	{ .crc = 0xE1714A88, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Asterix and the Secret Mission (E) [!].sms
	{ .crc = 0xDEF9B14E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Asterix (E) (V1.0) [!].sms
	{ .crc = 0x147E02FA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Asterix (E) (V1.1).sms
	{ .crc = 0x8C9D5BE8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior & Pit Pot (E) [!].sms
	{ .crc = 0x69EFD483, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior & Pit Pot (E) [b1][o1].sms
	{ .crc = 0x6A9EAEF7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior & Pit Pot (E) [o1].sms
	{ .crc = 0xCA558CA8, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior (U) [!].sms
	{ .crc = 0x299CBB74, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior (U) [b1].sms
	{ .crc = 0xBBE02270, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior (U) [b1][o1].sms
	{ .crc = 0xE31CE477, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Warrior (U) [b2].sms
	{ .crc = 0x4184E4C4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sapo Xule SOS Lagoa Poluida (B) [!].sms
	{ .crc = 0x7AB2946A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ayrton Senna's Super Monaco GP II (E) [!].sms
	{ .crc = 0xE890331D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aztec Adventure - The Golden Road to Paradise (UE) [!].sms
	{ .crc = 0xFF614EB3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aztec Adventure - The Golden Road to Paradise (UE) [o1].sms
	{ .crc = 0x95D86B45, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Back to the Future Part II (UE) [!].sms
	{ .crc = 0xE5FF50D8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Back to the Future Part III (UE) [!].sms
	{ .crc = 0x2D48C1D3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Baku Baku Animals (B) [!].sms
	{ .crc = 0x35D84DC2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bank Panic (E) [!].sms
	{ .crc = 0x0D3D510B, .rom = 0x8200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bank Panic (E) [p1][!].sms
	{ .crc = 0xB4DFB825, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Basket Ball Nightmare (E) [!].sms
	{ .crc = 0x0DF8597F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Basket Ball Nightmare (E) [b1].sms
	{ .crc = 0x7163231C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Basket Ball Nightmare (E) [b2].sms
	{ .crc = 0xCF97932D, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Batman Returns (UE) [!].sms
	{ .crc = 0xB154EC38, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Battle Out Run (E) [!].sms
	{ .crc = 0xC19430CE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Battlemaniacs (B) [!].sms
	{ .crc = 0x1CBB7BF1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Black Belt (UE) [!].sms
	{ .crc = 0xDA3A2F57, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Black Belt (UE) [o1].sms
	{ .crc = 0x8231C5E6, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hokuto no Ken (J) [!].sms
	{ .crc = 0x24F5FE8C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hokuto no Ken (J) [p1][!].sms
	{ .crc = 0x656D1A3E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blade Eagle 3D (UE) [!].sms
	{ .crc = 0x8ECD201C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blade Eagle 3D (UE) [T+Bra].sms
	{ .crc = 0xFBF96C81, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blade Eagle 3D (UE) [T+Bra_TMT].sms
	{ .crc = 0xFC259738, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bomber Raid (UE) [!].sms
	{ .crc = 0x3084CF11, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bomber Raid (UE) [b1].sms
	{ .crc = 0x8A3ED944, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bomber Raid (UE) [T+Bra_TMT].sms
	{ .crc = 0xE5FA2FAB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bonanza Bros (E) [!].sms
	{ .crc = 0xD778F3FE, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bonkers Wax Up! (UE) [!].sms
	{ .crc = 0xB3768A7A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bram Stoker's Dracula (E) [!].sms
	{ .crc = 0x1B10A951, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bram Stoker's Dracula (E) [T+Bra_TransCenter].sms
	{ .crc = 0x47C05B0C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bubble Bobble (E) [!].sms
	{ .crc = 0xE843BA7E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bubble Bobble (E) [T+Fre].sms
	{ .crc = 0xF7294EE1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Final Bubble Bobble (J) [!].sms
	{ .crc = 0x3EBB7457, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Final Bubble Bobble (J) [T+Spa100_pkt].sms
	{ .crc = 0x86D6384E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Buggy Run (E) [!].sms
	{ .crc = 0xB0FC4577, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games II (E) [!].sms
	{ .crc = 0xC0E25D62, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games II (E) [b1].sms
	{ .crc = 0x5F5D13AF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jogos de Verao II (B) [!].sms
	{ .crc = 0x45C50294, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games (UE) [!].sms
	{ .crc = 0xAC6009A7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games (UE) [b1].sms
	{ .crc = 0x13D49D42, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games (UE) [o1].sms
	{ .crc = 0x9F28E5F4, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games (UE) [T+Bra_Emuboarding].sms
	{ .crc = 0xB3B5D047, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// California Games (UE) [T+Bra_TMT].sms
	{ .crc = 0x86C1320A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Game Box Esportes Radicais (B) [!].sms
	{ .crc = 0x1890F407, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Captain Silver (E) [!].sms
	{ .crc = 0xA4852757, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Captain Silver (E) [b1].sms
	{ .crc = 0x87BF8A47, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Captain Silver (U) [!].sms
	{ .crc = 0xB81F6FA5, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Casino Games (UE) [!].sms
	{ .crc = 0x3CFF6E80, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Casino Games (UE) [b1].sms
	{ .crc = 0xF964F356, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Casino Games (UE) [b2].sms
	{ .crc = 0xE239ED95, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castelo Ra Tin Bum (B) [!].sms
	{ .crc = 0x31FFD7C3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castelo Ra Tin Bum (B) [b1].sms
	{ .crc = 0xDCD38617, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.0) [!].sms
	{ .crc = 0xB9DB4282, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.0) [T+Fre_terminus].sms
	{ .crc = 0x21A5434F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.0) [T+Ita0.99_darq][b1].sms
	{ .crc = 0x7A47AE9F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.1) [!].sms
	{ .crc = 0x953F42E1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.1) [b1].sms
	{ .crc = 0x6203EFBB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Castle of Illusion Starring Mickey Mouse (U) (V1.1) [T+Ita0.99_darq].sms
	{ .crc = 0x56A3AEFC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Champions of Europe (E) [!].sms
	{ .crc = 0x23163A12, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Championship Hockey (E) [!].sms
	{ .crc = 0x7E5839A0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cheese Cat-astrophe (E) [!].sms
	{ .crc = 0x46340C41, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cheese Cat-astrophe (E) [b1].sms
	{ .crc = 0x4BF77EC9, .rom = 0x6B7C8, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (J) (Prototype).sms
	{ .crc = 0x16EC3575, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [!].sms
	{ .crc = 0x4A21C15F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [b1].sms
	{ .crc = 0xC96FCD19, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [b1][o1].sms
	{ .crc = 0x75EB037D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [b2].sms
	{ .crc = 0xDEBB2E3A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [b3].sms
	{ .crc = 0xE0FDBC32, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Choplifter (UE) [o1].sms
	{ .crc = 0x9190A56F, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chuck Rock II - Son of Chuck (B) [!].sms
	{ .crc = 0x87783C04, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chuck Rock II - Son of Chuck (E) [!].sms
	{ .crc = 0xC30E690A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chuck Rock (E) [!].sms
	{ .crc = 0xDD0E2927, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cloud Master (UE) [!].sms
	{ .crc = 0xE7F62E6D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cloud Master (UE) [b1].sms
	{ .crc = 0x13D762EB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cloud Master (UE) [o1].sms
	{ .crc = 0xAA21BDF1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cloud Master (UE) [T+Fre100].sms
	{ .crc = 0x84E45C25, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Columns (UE) [!].sms
	{ .crc = 0x665FDA92, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cool Spot (E) [!].sms
	{ .crc = 0x13AC9023, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cool Spot (E) [b1].sms
	{ .crc = 0xCC92058A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cosmic Spacehead (E) [!].sms
	{ .crc = 0x29822980, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_SMS },
	// Cyber Shinobi (UE) [!].sms
	{ .crc = 0x1350E4F8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cyber Shinobi (UE) [T+Fre].sms
	{ .crc = 0xC576C058, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chouon Senshi Borgman (J) [!].sms
	{ .crc = 0xE421E466, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cyborg Hunter (UE) [!].sms
	{ .crc = 0x908E7524, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cyborg Hunter (UE) [T+Bra].sms
	{ .crc = 0x903AAD36, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cyborg Hunter (UE) [T+Fre_Terminus].sms
	{ .crc = 0x0F2E4F1E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Cyborg Hunter (UE) [T+Ita1.00].sms
	{ .crc = 0xBDD034EE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Daffy Duck in Hollywood (E) [!].sms
	{ .crc = 0x71ABEF27, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Danan the Jungle Fighter (E) [!].sms
	{ .crc = 0xAE4A28D7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dead Angle (UE) [!].sms
	{ .crc = 0xE2F7193E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dead Angle (UE) [b1].sms
	{ .crc = 0x5EF8DBDA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Deep Duck Trouble (E) [!].sms
	{ .crc = 0x42FC3A6E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Desert Speedtrap - Starring Road Runner and Wile E. Coyote (E) (M5).sms
	{ .crc = 0xB137007A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Desert Strike (UE) [!].sms
	{ .crc = 0x6C1433F9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Desert Strike (UE) [b1].sms
	{ .crc = 0xDF8C8F76, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Desert Strike (UE) [b2].sms
	{ .crc = 0x82EFDA3C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dick Tracy (UE) [!].sms
	{ .crc = 0xF6FAB48D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dinobasher - Starring Bignose the Caveman (Prototype) [!].sms
	{ .crc = 0xEA5C3A6F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dodgeball King (K).sms
	{ .crc = 0x89B79E77, .rom = 0x6C000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Dragon (UE) [!].sms
	{ .crc = 0xB8CFFA0F, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Dragon (UE) [b1].sms
	{ .crc = 0xFAA2C8DB, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Dragon (UE) [b2].sms
	{ .crc = 0x3B79D63E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Dragon (UE) [b3].sms
	{ .crc = 0x6CC2E967, .rom = 0x40400, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Hawk (E) [!].sms
	{ .crc = 0x8370F6CD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Hawk (E) [b1].sms
	{ .crc = 0x52D9EAF1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dr. HELLO (K).sms
	{ .crc = 0x16537865, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dr. Robotnik's Mean Bean Machine (E) [!].sms
	{ .crc = 0x6C696221, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dragon - The Bruce Lee Story (E) [!].sms
	{ .crc = 0xC88A5064, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dragon - The Bruce Lee Story (E) [T+Bra99%_Guto].sms
	{ .crc = 0xC81C3061, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dragon Crystal (E) [!].sms
	{ .crc = 0x9549FCE4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dynamite Duke (E) [!].sms
	{ .crc = 0x07306947, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dynamite Duke (E) [b1].sms
	{ .crc = 0x2BB5F2CD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dynamite Dux (E) [!].sms
	{ .crc = 0x0E1CC1E0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Dynamite Headdy (B) [!].sms
	{ .crc = 0x7DB5B0FA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// E-SWAT - City Under Siege (UE) [!].sms
	{ .crc = 0xC4BB1676, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// E-SWAT - City Under Siege (UE) [a1][!].sms
	{ .crc = 0xC10FCE39, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Earthworm Jim (B) [!].sms
	{ .crc = 0xC4D5EFC5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ecco the Dolphin - Tides of Time (B).sms
	{ .crc = 0x7C28703A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ecco the Dolphin (UE).sms
	{ .crc = 0x6687FAB9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Enduro Racer (J).sms
	{ .crc = 0x5D5C50B3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Enduro Racer (UE) [!].sms
	{ .crc = 0x00E73541, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Enduro Racer (UE) [b1].sms
	{ .crc = 0xDFA2AE07, .rom = 0x1FFFF, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Enduro Racer (UE) [T+Bra].sms
	{ .crc = 0x1FE4E74D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// F-16 Fighter (E) [!].sms
	{ .crc = 0xEAEBF323, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// F-16 Fighting Falcon (J) [!].sms
	{ .crc = 0x7CE06FCE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// F-16 Fighting Falcon (U) [!].sms
	{ .crc = 0x184C23B7, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// F1 Championship (E) [!].sms
	{ .crc = 0xEC788661, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// FIFA International Soccer (B) [!].sms
	{ .crc = 0x9BB3B5F9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantastic Dizzy (E) [!].sms
	{ .crc = 0xB9664AE1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_SMS },
	// Fantasy Zone - The Maze (UE) [!].sms
	{ .crc = 0xD29889AD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone - The Maze (UE) [o1].sms
	{ .crc = 0x483394F9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Opa Opa (J).sms
	{ .crc = 0xBEA27D5C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone II - The Tears of Opa-Opa (J) [!].sms
	{ .crc = 0xC722FB42, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone II - The Tears of Opa-Opa (UE) [!].sms
	{ .crc = 0xA5233205, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone II - The Tears of Opa-Opa (UE) [b1].sms
	{ .crc = 0xF7EAF75A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone (UE) [!].sms
	{ .crc = 0x65D7E4E0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone (UE) [o1].sms
	{ .crc = 0x82A38EF8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fantasy Zone (UE) [T+Fre_GIX].sms
	{ .crc = 0x0DF8D42E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ferias Frustradas do Picapau (B).sms
	{ .crc = 0xBF6C2E37, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fire & Forget 2 (E) [!].sms
	{ .crc = 0xF6AD7B1D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fire & Ice (B) [!].sms
	{ .crc = 0x8B24A640, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Flash, The (E) [!].sms
	{ .crc = 0xBE31D63F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Flicky (SG-1000) [b1].sms
	{ .crc = 0xD4C8C6D7, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Flintstones, The (E) [!].sms
	{ .crc = 0xCA5C78A5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Forgotten Worlds (E) [!].sms
	{ .crc = 0x38C53916, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fushigi no Oshiro Pit Pot (J) [!].sms
	{ .crc = 0xE6795C53, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fushigi no Oshiro Pit Pot (J) [p1][!].sms
	{ .crc = 0x5D08E823, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// G-Loc Air Battle (E) [!].sms
	{ .crc = 0x05CDC24E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// GP Rider (U) [!].sms
	{ .crc = 0xEC2DA554, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Gain Ground (UE) [!].sms
	{ .crc = 0x3EC5E627, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Galactic Protector (UE) [!].sms
	{ .crc = 0xA6FA42D0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Galaxy Force (E) [!].sms
	{ .crc = 0xA4AC35D8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Galaxy Force (U) [!].sms
	{ .crc = 0x6C827520, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Galaxy Force (U) [b1].sms
	{ .crc = 0x1820C8AC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Gangster Town (UE) [!].sms
	{ .crc = 0x5FC74D2A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Gauntlet (UE) [!].sms
	{ .crc = 0xD9190956, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Gauntlet (UE) [b1].sms
	{ .crc = 0xAF2ABB95, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// George Foreman's KO Boxing (E) [!].sms
	{ .crc = 0xA64898CE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chapolim x Dracula - Um Duelo Assustador (B) [!].sms
	{ .crc = 0x492C7C6E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (J) (Prototype).sms
	{ .crc = 0xC3E7C1ED, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (J) [!].sms
	{ .crc = 0xC0F3CE7E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (J) [p1][!].sms
	{ .crc = 0xDABCC054, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [!].sms
	{ .crc = 0xF1F8FF2D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [b1].sms
	{ .crc = 0x51D7E0B5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [o1].sms
	{ .crc = 0xF34FE006, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [o2].sms
	{ .crc = 0xD3E6EAA6, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [o3].sms
	{ .crc = 0x3A020F2B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [o4].sms
	{ .crc = 0x61BC1FA6, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghost House (UE) [T+Bra_Leo].sms
	{ .crc = 0x9A1ECFEC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghostbusters (UE) [!].sms
	{ .crc = 0x1DDC3059, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghouls 'n Ghosts (UE) [!].sms
	{ .crc = 0x6700985A, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghouls 'n Ghosts (UE) [T+Bra][p1][!].sms
	{ .crc = 0xDB48B5EC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ghouls 'n Ghosts (UE) [T+Bra_Guto].sms
	{ .crc = 0xCFAE7E9B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Global Defense (Prototype).sms
	{ .crc = 0x91A0FC4E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Global Defense (UE) [!].sms
	{ .crc = 0xB746A6F5, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Global Defense (UE) [o1].sms
	{ .crc = 0x75F61887, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SDI (J) [!].sms
	{ .crc = 0x1DE2C2D0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [!].sms
	{ .crc = 0xC7DED988, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Bra].sms
	{ .crc = 0xF424AD15, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Bra_ALVS].sms
	{ .crc = 0x68F58DF7, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Bra_Emuboarding].sms
	{ .crc = 0x8872F23F, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Fre_Haruney].sms
	{ .crc = 0x472D1CE4, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Ger0.82_Taurus].sms
	{ .crc = 0x21DB20AE, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe Warrior (UE) [T+Spa100_pkt].sms
	{ .crc = 0xA53677B3, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe (UE) [!].sms
	{ .crc = 0x4B980452, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golden Axe (UE) [b1].sms
	{ .crc = 0x95F54C15, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golf Mania (E) [!].sms
	{ .crc = 0x48651325, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golf Mania (Prototype) [!].sms
	{ .crc = 0x5DABFDC3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golvellius - Valley of Doom (J).sms
	{ .crc = 0xBF0411AD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golvellius - Valley of Doom (UE) [!].sms
	{ .crc = 0xA51376FE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golvellius - Valley of Doom (UE) [T+Bra].sms
	{ .crc = 0x00196EBA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golvellius - Valley of Doom (UE) [T+Fre_floflo].sms
	{ .crc = 0x11E19FB1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Golvellius - Valley of Doom (UE) [T+Ger1.00_Star-trans].sms
	{ .crc = 0x359141EF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Baseball (J) [!].sms
	{ .crc = 0x89E98A7C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Baseball (J) [p1][!].sms
	{ .crc = 0xC1E699DB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Baseball (UE) [!].sms
	{ .crc = 0x10ED6B57, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Basketball (UEB) [!].sms
	{ .crc = 0x2AC001EB, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Football (UE) [!].sms
	{ .crc = 0x2055825F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Football (UE) [o1].sms
	{ .crc = 0x1533B4ED, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (J) (Prototype).sms
	{ .crc = 0x4847BC91, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (J) [!].sms
	{ .crc = 0x6586BD1F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (UE) (V1.0) [!].sms
	{ .crc = 0xC6611C84, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (UE) (V1.1) [!].sms
	{ .crc = 0x98E4AE4A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (UE) (V1.1) [o1].sms
	{ .crc = 0x00B31658, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Golf (UE) (V1.1) [o2].sms
	{ .crc = 0x94B4E09A, .rom = 0x2E5C8, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Ice Hockey (UE) [!].sms
	{ .crc = 0x0CB7E21F, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Ice Hockey (UE) [o1].sms
	{ .crc = 0x946B8C4A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Soccer (E).sms
	{ .crc = 0x0ED170C9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Soccer (J) [!].sms
	{ .crc = 0x2D7FD7EF, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Soccer (J) [p1][!].sms
	{ .crc = 0x84665648, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Volleyball (J) [!].sms
	{ .crc = 0x6819B0C0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Volleyball (UE) [!].sms
	{ .crc = 0x8D43EA95, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Great Volleyball (UE) [b1].sms
	{ .crc = 0xB2801B0C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Guzzler (MV) [!].mv
	{ .crc = 0x61FA9EA0, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On & Astro Warrior (U) [!].sms
	{ .crc = 0x1C5059F0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On & Safari Hunt (U) [!].sms
	{ .crc = 0xE167A561, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On & Safari Hunt (U) [b1].sms
	{ .crc = 0xC5083000, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On (E) [!].sms
	{ .crc = 0x6F79E4A1, .rom = 0x8200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On (E) [T+Bra].sms
	{ .crc = 0x585C448A, .rom = 0x8200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On (E) [T-Bra].sms
	{ .crc = 0xA77F996F, .rom = 0x8200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On (E) [T-Bra_TMT][b1].sms
	{ .crc = 0xBCD19BC6, .rom = 0x81F8, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hang-On (J) [!].sms
	{ .crc = 0x5C01ADF9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Heroes of the Lance (U) [!].sms
	{ .crc = 0xCDE13FFB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// High School! Kimengumi (J) [!].sms
	{ .crc = 0x9EB1AA4F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// High School! Kimengumi (J) [T+Eng].sms
	{ .crc = 0x8A296A3E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// High School! Kimengumi (J) [T-EngBeta].sms
	{ .crc = 0x6F30434B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Home Alone (U) [!].sms
	{ .crc = 0xC9DBF936, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hook (Prototype) [!].sms
	{ .crc = 0x9CED34A7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hook (Prototype) [T+Bra_ALVS].sms
	{ .crc = 0x95256919, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hoshi wo Sagasite... (J) [!].sms
	{ .crc = 0x955A009E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Impossible Mission (E) [!].sms
	{ .crc = 0x64D6AF3B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Incredible Crash Dummies, The (E) [!].sms
	{ .crc = 0xB4584DDE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Incredible Hulk, The (E) [!].sms
	{ .crc = 0xBE9A7071, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Indiana Jones and the Last Crusade (UE) [!].sms
	{ .crc = 0x8AEB574B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Heavyweight Champ (E) [!].sms
	{ .crc = 0xFDAB876A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James 'Buster' Douglas Knockout Boxing (U) [!].sms
	{ .crc = 0x6A664405, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James Bond 007 - The Duel (B) [!].sms
	{ .crc = 0x8FEFF688, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James Bond 007 - The Duel (B) [b1].sms
	{ .crc = 0x2B7F7447, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James Bond 007 - The Duel (E).sms
	{ .crc = 0x8D23587F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James Pond 2 - Codename Robocod (UE) [!].sms
	{ .crc = 0x102D5FEA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// James Pond 2 - Codename Robocod (UE) [b1].sms
	{ .crc = 0xFEF68275, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Joe Montana Football (U) [!].sms
	{ .crc = 0x0A9089E5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jungle Book, The (UE) [!].sms
	{ .crc = 0x695A9A15, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jungle Book, The (UE) [b1].sms
	{ .crc = 0xE959E820, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jungle Book, The (UE) [T+Bra][p1][!].sms
	{ .crc = 0xDCA0C49E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jungle Book, The (UE) [T+Bra_Emubrazil].sms
	{ .crc = 0x5CA3B6AB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jurassic Park (E) [!].sms
	{ .crc = 0x0667ED9F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (J) [!].sms
	{ .crc = 0x05EA5353, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [!].sms
	{ .crc = 0x4CFCA0D2, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [o1].sms
	{ .crc = 0x4CC95B51, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [T+Bra_ALVS].sms
	{ .crc = 0x548AA53C, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [T+Fre].sms
	{ .crc = 0xA28218FE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [T+Pol1.1_devilfox].sms
	{ .crc = 0xEB032DB7, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kenseiden (UE) [T-Pol1.0fix_devilfox].sms
	{ .crc = 0x7CFFC3A1, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// King's Quest - Quest for the Crown (Prototype).sms
	{ .crc = 0xB7FE0A9D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// King's Quest - Quest for the Crown (U) [!].sms
	{ .crc = 0xF8D33BC4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Klax (E) [!].sms
	{ .crc = 0x2B435FD6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Klax (E) [b1].sms
	{ .crc = 0xBB351118, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Klax (E) [b2].sms
	{ .crc = 0x97B38E8A, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Klax (E) [b3].sms
	{ .crc = 0xED15F21E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Krusty's Fun House (E) [!].sms
	{ .crc = 0x64A585EB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kung Fu Kid (UE) [!].sms
	{ .crc = 0x1E949D1F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kung Fu Kid (UE) [o1].sms
	{ .crc = 0x1FBE3106, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Makai Retsuden (J) [!].sms
	{ .crc = 0xCA860451, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sapo Xule O Mestre do Kung Fu (B) [!].sms
	{ .crc = 0x890E83E4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Land of Illusion Starring Mickey Mouse (U) [!].sms
	{ .crc = 0x24E97200, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Laser Ghost (E) [!].sms
	{ .crc = 0x0CA95637, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Laser Ghost (E) [b1].sms
	{ .crc = 0x33CF8213, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Legend of Illusion Starring Mickey Mouse (U) [!].sms
	{ .crc = 0x6350E649, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lemmings (E) [!].sms
	{ .crc = 0xF369B2D8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lemmings (Prototype) [!].sms
	{ .crc = 0x2C61ED88, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Line of Fire (E) [!].sms
	{ .crc = 0xCB09F355, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lion King, The (E) [!].sms
	{ .crc = 0xC352C7EB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Loletta no Syouzou (J) [!].sms
	{ .crc = 0x323F357F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lord of the Sword (J) [!].sms
	{ .crc = 0xAA7D6F45, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lord of the Sword (UE) [!].sms
	{ .crc = 0xF5C368F4, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lord of the Sword (UE) [b1].sms
	{ .crc = 0x3D87705B, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lord of the Sword (UE) [b2].sms
	{ .crc = 0x2C3FA788, .rom = 0x40022, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lucky Dime Caper, The - Starring Donald Duck (E) [!].sms
	{ .crc = 0xA1710F13, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lucky Dime Caper, The - Starring Donald Duck (E) [T+Bra].sms
	{ .crc = 0xD79E51A8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lucky Dime Caper, The - Starring Donald Duck (E) [T+Fre_GIX].sms
	{ .crc = 0x47340BE0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Lucky Dime Caper, The - Starring Donald Duck (Prototype).sms
	{ .crc = 0x7F6D0DF6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Machine Gun Joe (J) [!].sms
	{ .crc = 0x9D549E08, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mahjong Sengoku Jidai (J) [!].sms
	{ .crc = 0xBCFBFC67, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mahjong Sengoku Jidai (J) [b1][o1].sms
	{ .crc = 0x3FAF4E19, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mahjong Sengoku Jidai (J) [o1].sms
	{ .crc = 0x1A9B396F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mahjong Sengoku Jidai (Prototype).sms
	{ .crc = 0x996B2A07, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Marble Madness (UE) [!].sms
	{ .crc = 0xBF6F3E5F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Marble Madness (UE) [b1].sms
	{ .crc = 0x94FEE07B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Marble Madness (UE) [T+Spa100_sinister].sms
	{ .crc = 0xD4F59AFB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Marksman Shooting - Trap Shooting (U) [!].sms
	{ .crc = 0xE8EA842C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master of Darkness (UE) [!].sms
	{ .crc = 0x96FB4D4B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master of Darkness (UE) [b1].sms
	{ .crc = 0x49C5D8E2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vampire (E) [!].sms
	{ .crc = 0x20F40CAE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vampire (E) [b1].sms
	{ .crc = 0x39540619, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vampire (E) [T+Fre1.00_Terminus].sms
	{ .crc = 0x720B6973, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Masters of Combat (E) [!].sms
	{ .crc = 0x93141463, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Masters of Combat (E) [b1].sms
	{ .crc = 0x22F2C541, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Maze Hunter 3D (U) [!].sms
	{ .crc = 0x31B8040B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Maze Walker (J) [!].sms
	{ .crc = 0x871562B0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Megumi Rescue (J) [!].sms
	{ .crc = 0x29BC7FAD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mercs (E) [!].sms
	{ .crc = 0xD7416B83, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Michael Jackson's Moonwalker (UE) [!].sms
	{ .crc = 0x56CC906B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Michael Jackson's Moonwalker (UE) [b1].sms
	{ .crc = 0x9C4F03AC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mick & Mack as The Global Gladiators (E) [!].sms
	{ .crc = 0xB67CEB76, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mick & Mack as The Global Gladiators (E) [o1].sms
	{ .crc = 0x8DAE576E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mick & Mack as The Global Gladiators (E) [o1][b1].sms
	{ .crc = 0x87475035, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mick & Mack as The Global Gladiators (E) [t1].sms
	{ .crc = 0x619FDE2C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mickey's Ultimate Challenge (B) [!].sms
	{ .crc = 0x25051DD5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mickey's Ultimate Challenge (B) [b1].sms
	{ .crc = 0x80F94F99, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mickey's Ultimate Challenge (B) [o1].sms
	{ .crc = 0x2B86BCD7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mickey's Ultimate Challenge (B) [o2].sms
	{ .crc = 0x7E811F8D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Micro Machines (E) [!].sms
	{ .crc = 0xA577CE46, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_SMS },
	// Micro Machines (E) [b1].sms
	{ .crc = 0x58FA27C6, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_SMS },
	// Haja no Fuuin (J) [!].sms
	{ .crc = 0xB9FDF6D9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Miracle Warriors - Seal of the Dark Lord (UE) [!].sms
	{ .crc = 0x0E333B6E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Miracle Warriors - Seal of the Dark Lord (UE) [b1].sms
	{ .crc = 0xEBC2BFA1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Miracle Warriors - Seal of the Dark Lord (UE) [b2].sms
	{ .crc = 0xF14A8546, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Missile Defense 3-D (UE) [!].sms
	{ .crc = 0xFBE5CFBB, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monopoly (E) [!].sms
	{ .crc = 0x026D94A4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monopoly (U) [!].sms
	{ .crc = 0x69538469, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monopoly (U) [o1].sms
	{ .crc = 0xFFF835BD, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Montezuma's Revenge (U) [!].sms
	{ .crc = 0x82FDA895, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mortal Kombat 2 (E) [!].sms
	{ .crc = 0x2663BF18, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mortal Kombat 2 (E) [b1].sms
	{ .crc = 0x9ED8341A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mortal Kombat 3 (E) [!].sms
	{ .crc = 0x395AE757, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mortal Kombat (E) [!].sms
	{ .crc = 0x302DC686, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Mortal Kombat (E) [T+Bra_Guto].sms
	{ .crc = 0x563F2487, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ms. Pac-man (E) [!].sms
	{ .crc = 0x3CD816C6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// My Hero (UE) [!].sms
	{ .crc = 0x62F0C23D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// My Hero (UE) [o1].sms
	{ .crc = 0x8E9B0F4B, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// My Hero (UE) [o2].sms
	{ .crc = 0x99985AD9, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// My Hero (UE) [o3].sms
	{ .crc = 0x144CFA14, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Seishyun Scandal (J) [!].sms
	{ .crc = 0xF0BA2BC6, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Seishyun Scandal (J) [b1].sms
	{ .crc = 0x56476A54, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Seishyun Scandal (J) [p1][!].sms
	{ .crc = 0xBCD91D78, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Nekyuu Kousien (J) [!].sms
	{ .crc = 0x5B5F9106, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// New Zealand Story, The (E) [!].sms
	{ .crc = 0xC660FF34, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// New Zealand Story, The (E) [b1].sms
	{ .crc = 0xFBC66CBF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Nihonshi Ninpyou (SC-3000) [!].sc
	{ .crc = 0x22F4F92A, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (E) [!].sms
	{ .crc = 0x1B1D8CC2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (E) [T+Fre_Terminus].sms
	{ .crc = 0x4CD38BE4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (E) [T+Ger].sms
	{ .crc = 0xC28E542F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (E) [T+Spa100_Pkt].sms
	{ .crc = 0xC94C2B4E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [!].sms
	{ .crc = 0x761E9396, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [T+Fre].sms
	{ .crc = 0x0643620E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [T+Fre][b1].sms
	{ .crc = 0x9D5A2845, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [T+Fre_Terminus].sms
	{ .crc = 0x41E5B485, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [T+Ger1.00_Star-trans].sms
	{ .crc = 0x317F14DA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja Gaiden (Prototype) [T+Spa100_Pkt].sms
	{ .crc = 0x17FEE94A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja, The (J) [!].sms
	{ .crc = 0x320313EC, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja, The (UE) [!].sms
	{ .crc = 0x66A15BD9, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja, The (UE) [o1].sms
	{ .crc = 0x2E0A57D6, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ninja, The (UE) [o2].sms
	{ .crc = 0xCF9F0A7F, .rom = 0x40040, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Operation Wolf (U) [!].sms
	{ .crc = 0x205CAAE8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Operation Wolf (U) [o1].sms
	{ .crc = 0xD85A05E8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Operation Wolf (U) [T+Fre].sms
	{ .crc = 0x23283F37, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ottifants, The (E) [!].sms
	{ .crc = 0x82EF2A7D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun 3D (E) [!].sms
	{ .crc = 0xD6F43DDA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun Europa (E) [!].sms
	{ .crc = 0x3932ADBC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun (UE) [!].sms
	{ .crc = 0x481BAB2E, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun (UE) [b1].sms
	{ .crc = 0x04FA0682, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun (UE) [b2].sms
	{ .crc = 0xCA99CEFA, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// OutRun (UE) [o1].sms
	{ .crc = 0xDC0860E7, .rom = 0x40040, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// PGA Tour Golf (E) [!].sms
	{ .crc = 0x95B9EA95, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pac-Mania (E) [!].sms
	{ .crc = 0xBE57A9A5, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pac-Mania (E) [b1].sms
	{ .crc = 0x2BE9619B, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pac-Mania (E) [o1].sms
	{ .crc = 0x6708CC37, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Paperboy (E) [!].sms
	{ .crc = 0x294E0759, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Paperboy (E) [t1].sms
	{ .crc = 0xC0E1EBC4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Paperboy (U) [!].sms
	{ .crc = 0x327A0B4C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Paperboy (U) [t1].sms
	{ .crc = 0x36F04D14, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Parlour Games (UE) [!].sms
	{ .crc = 0xE030E66C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Party Games (J) [!].sms
	{ .crc = 0x7ABC70E9, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pat Riley Basketball (Prototype) [!].sms
	{ .crc = 0x9AEFE664, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Penguin Land (J) [!].sms
	{ .crc = 0x2BCDB8FA, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Penguin Land (SG-1000) [b1].sms
	{ .crc = 0xAC7F95B4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Penguin Land (UE) [!].sms
	{ .crc = 0xF97E9875, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Penguin Land (UE) [o1].sms
	{ .crc = 0xF6552DA8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (B) [!].sms
	{ .crc = 0x75971BEF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (J) (from Saturn Collection CD) [!].sms
	{ .crc = 0x07301F83, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (J) [!].sms
	{ .crc = 0x6605D36A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (J) [b1].sms
	{ .crc = 0xEEFE22DE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (K) [!].sms
	{ .crc = 0x747E83B5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (Lutz Hack).sms
	{ .crc = 0x3CA83C04, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (UE) (V1.2) [!].sms
	{ .crc = 0xE4A65E79, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (UE) (V1.3) [!].sms
	{ .crc = 0x00BEF1D7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (UE) (V1.3) [b1].sms
	{ .crc = 0x7F4F28C6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (UE) (V1.3) [b2].sms
	{ .crc = 0xE80EE900, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Phantasy Star (UE) (V1.3) [T+Fre_floflo].sms
	{ .crc = 0x56BD28D8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pit Fighter (B).sms
	{ .crc = 0xAA4D4B5A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pit Fighter (UE) [!].sms
	{ .crc = 0xB840A446, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pit Fighter (UE) [b1].sms
	{ .crc = 0x283712A9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Populous (E) [!].sms
	{ .crc = 0xC7A1FDEF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Populous (E) [b1].sms
	{ .crc = 0x4A9B4ECB, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Poseidon Wars 3D (UE) [!].sms
	{ .crc = 0xABD48AD2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Power Strike 2 (E) [!].sms
	{ .crc = 0xA109A6FE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Power Strike 2 (E) [b1].sms
	{ .crc = 0x673C68B8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Aleste (W) [!].sms
	{ .crc = 0xD8C4165B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Power Strike (UE) [!].sms
	{ .crc = 0x4077EFD9, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Power Strike (UE) [o1].sms
	{ .crc = 0x4CDC0942, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Predator 2 (E) [!].sms
	{ .crc = 0x0047B615, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Prince of Persia (E).sms
	{ .crc = 0x7704287D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Gokuaku Doumei Dump Matsumoto (J) [!].sms
	{ .crc = 0xA249FA9D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pro Wrestling (UE) [!].sms
	{ .crc = 0xFBDE42D3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pro Wrestling (UE) [b1].sms
	{ .crc = 0xFD069AAE, .rom = 0x3E000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pro Wrestling (UE) [o1].sms
	{ .crc = 0x09489195, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pro Yakyuu Pennant Race, The (J) [!].sms
	{ .crc = 0xDA9BE8F0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Promocao Especial M.System III Compact (B) [!].sms
	{ .crc = 0x30AF0233, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psychic World (E) [!].sms
	{ .crc = 0x5C0B1F0F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psychic World (E) [T+Fre].sms
	{ .crc = 0x255C2560, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psychic World (E) [T+Ger1.00_Star-trans].sms
	{ .crc = 0x9B700645, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psychic World (E) [T+Ita1.00_darq].sms
	{ .crc = 0xD155C2F8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jornada do Mymo (Psycho Fox Hack) [a1].sms
	{ .crc = 0xBBC237C0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jornada do Mymo (Psycho Fox Hack) [b1].sms
	{ .crc = 0xE05E3D2B, .rom = 0x25088, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jornada do Mymo (Psycho Fox Hack).sms
	{ .crc = 0x5D6DEC5E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Jornada do Mymo II (Psycho Fox Hack).sms
	{ .crc = 0x16C217C9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Smash Brothers V1.0 (Psycho Fox Hack).sms
	{ .crc = 0x1C04A0BC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Natal do Mymo (Psycho Fox Hack).sms
	{ .crc = 0x02BD3A57, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pokemon Master V2.0 (Psycho Fox Hack).sms
	{ .crc = 0xD3AA5894, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psycho Fox (UE) [!].sms
	{ .crc = 0x97993479, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psycho Fox (UE) [T+Bra_Tamanduon].sms
	{ .crc = 0x92D2C75B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Psycho Fox (UE) [T+Fre_Terminus].sms
	{ .crc = 0xB8E2FEBC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R.H.W. 2000 (Psycho Fox Hack).sms
	{ .crc = 0xC804943F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sapo Xule vs. os Invasores do Brejo (B).sms
	{ .crc = 0x9A608327, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic vs. os Mauniks (Psycho Fox Hack).sms
	{ .crc = 0xA6933E55, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Treinamento Do Mymo (B).sms
	{ .crc = 0xE94784F2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// #gscept Intro by blindio (PD).sms
	{ .crc = 0xD87316F6, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// 64 Color Palette Test (PD).sms
	{ .crc = 0xA581402E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// AntiISDA Warrior by Ventzislav Tzvetkov (V1.00) (PD).sms
	{ .crc = 0xCA270F40, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// AntiISDA Warrior by Ventzislav Tzvetkov (V1.01) (PD).sms
	{ .crc = 0x7408EF2F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// AntiISDA Warrior by Ventzislav Tzvetkov (V1.02) (PD).sms
	{ .crc = 0xB3E5986E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Artillery Master 8k V0.9 by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x66C125F1, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blockhead v20040714 by Proppy & Tet (PD).sms
	{ .crc = 0x6994770D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blockhead v20040807 by Proppy & Tet (PD).sms
	{ .crc = 0xAED594A2, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blockhead v20041016 by Proppy & Tet (PD).sms
	{ .crc = 0xA11B791C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blockhead WIP1 by Proppy & Tet (PD).sms
	{ .crc = 0x79A1D911, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Blockhead WIP2 by Proppy & Tet (PD).sms
	{ .crc = 0x2734A9B4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// BMP2Tile Demo (PD).sms
	{ .crc = 0x7694ED1C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bock's Birthday 2002 by Maxim (PD).sms
	{ .crc = 0x1D43A351, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bock's Birthday 2003 by Maxim (PD).sms
	{ .crc = 0xC07CB738, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bock's Birthday 2004 by xiao (PD).sms
	{ .crc = 0x31A5CA6A, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Bock's Birthday 2006 by Charles MacDonald (PD).sms
	{ .crc = 0xF0F53C64, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Canyon Racer V0.01alpha by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0xEEAD16C7, .rom = 0x1DCA, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Chkstate Demo by Dave (PD).sms
	{ .crc = 0x5CB841DD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Code 38 - Volume I (PD).sms
	{ .crc = 0xBFEE193C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Color & Switch Test (PD).sms
	{ .crc = 0x7253C3EC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Copyright Violation by Nicolas Warren (PD).sms
	{ .crc = 0x43600BB1, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Damiana (PD).sms
	{ .crc = 0x97F8DCC8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// DCEvolution.net Intro (PD).sms
	{ .crc = 0x7CC2A20B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Digger Chan by Aypok (PD).sms
	{ .crc = 0xC4CA6878, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Draw Poker by Mike Beaver (PD).sms
	{ .crc = 0xDDDB3DD8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Driar (V0.05) by fx and ejdolf (PD).sms
	{ .crc = 0x2E31ACD2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Elite Gaiden V0.1.8 by Ricardo Bittencourt (PD).sms
	{ .crc = 0x87AB46F8, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Fri For Fransk! V1.02 by Marc Klemp (PD).sms
	{ .crc = 0xA95013D3, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Genesis 6 Button Controller Test by Charles MacDonald (PD).sms
	{ .crc = 0x4FCC473B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// GoodAdvice by Nicolas Warren (PD).sms
	{ .crc = 0xE02FC973, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Happy Looser by Zoop (PD) [a1].sms
	{ .crc = 0x922954C6, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Happy Looser by Zoop (PD).sms
	{ .crc = 0x07C39BA4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Headbreak For A Scroller (PD).sms
	{ .crc = 0xED169D59, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Hello, World Test 1 (PD).sms
	{ .crc = 0x10CA9DD3, .rom = 0xE7E, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Interactive Sprite Test (PD).sms
	{ .crc = 0x191C3113, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Interrupt Test (PD).sms
	{ .crc = 0x6887900E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// KunKun & KokoKun by Bock (PD).sms
	{ .crc = 0x7E15A103, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Line Interrupt Test #1 by Charles MacDonald (PD).sms
	{ .crc = 0xC264580F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// NanoWars 8k v0.7 by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0xA06D065C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Nine Pixels by Nicolas Warren (PD).sms
	{ .crc = 0x82AE5ACD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Not Only Words (PD).sms
	{ .crc = 0x261587B8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Only Words by Mike Gordon (PD).sms
	{ .crc = 0x580293EE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Paws 32K Intro by An!mal (PD).sms
	{ .crc = 0x85060847, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// PFR Detect V1.00 by Eric R. Quinn (PD).sms
	{ .crc = 0xAD0FF4F1, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// PFR Detect V2.00 by Eric R. Quinn (PD).sms
	{ .crc = 0x1F30CB32, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Photo (PD).sms
	{ .crc = 0x0652B785, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pongmaster 4k by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0xF87D50FD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pongmaster by Haroldo O. Pinheiro (PD) [a1].sms
	{ .crc = 0xEF3D4C50, .rom = 0x1AB0, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pongmaster by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x3879AC1B, .rom = 0x16CC, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// River Strike V0.01alpha by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x53CB13C1, .rom = 0x1208, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// River Strike V0.02alpha by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x78DF344C, .rom = 0x1D07, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// River Strike V0.03alpha by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x767C6744, .rom = 0x20F4, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// River Strike V0.04alpha by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x1FE7F34E, .rom = 0x21B4, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Robbyie v0.999 (PD).sms
	{ .crc = 0xF61874C9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SALY V1.00 by Marc Klemp (PD).sms
	{ .crc = 0x7CBD4432, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Scroll Test Program by Charles MacDonald (PD).sms
	{ .crc = 0x6841CE1D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Mark III BG Test v0.01 (PD).sms
	{ .crc = 0x5F1120C8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Mark III Port Test v0.1 (PD).sms
	{ .crc = 0x49790A5A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Tween (3D) by Ben Ryves (PD).sms
	{ .crc = 0xB0131D77, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Tween (Normal) by Ben Ryves (PD).sms
	{ .crc = 0xF97C47A0, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SegaSlide by FluBBa (PD).sms
	{ .crc = 0x2A2AC7E1, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Simple Sprite Demo Release 2 by Nicolas Warren (PD).sms
	{ .crc = 0x5F40660D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS 3-D Demo by Chris Covell (PD).sms
	{ .crc = 0xA0440666, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS APA Demo V0.01 by Haroldo O. Pinheiro (PD).sms
	{ .crc = 0x350F7497, .rom = 0x3664, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Boot Loader 0.9 (PD).sms
	{ .crc = 0xAD31C5FF, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Cast - A Tribute To Loose Logic (PD).sms
	{ .crc = 0x97B89878, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Chip-8 V0.1 by Maxim (PD).sms
	{ .crc = 0x1E9FCE5F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Chip-8 V0.21 by Maxim (PD).sms
	{ .crc = 0x1ABF0172, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Chip-8 V1.11 by Maxim (PD).sms
	{ .crc = 0xA0154E51, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Palette by Omar Cornut (PD).sms
	{ .crc = 0xEA766665, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Power Demo Build 31 by Zoop (PD).sms
	{ .crc = 0x1A07B7A4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Power's 7th Anniversary Intro V1.00 by Nicolas Warren (PD).sms
	{ .crc = 0x297EFB87, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Power's 7th Anniversary Intro V1.03 by Nicolas Warren (PD) [a1].sms
	{ .crc = 0xA2FF5A9E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Power's 7th Anniversary Intro V1.03 by Nicolas Warren (PD).sms
	{ .crc = 0x72506A85, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Sprite Test (PD).sms
	{ .crc = 0xBD274327, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Tetris Release 2 by Nicolas Warren (PD).sms
	{ .crc = 0x983E17E4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Tetris Release 3 by Nicolas Warren (PD).sms
	{ .crc = 0xFD603AEA, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS Tetris Release 4 by Nicolas Warren (PD).sms
	{ .crc = 0xA0BF829B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMS VGM player V0.45 by Maxim (PD).sms
	{ .crc = 0x7DBDE57E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Basic Demo by Super Majik Spiral Crew (PD) [b1].sms
	{ .crc = 0xD654CFAD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Text Demo V1 by Super Majik Spiral Crew (PD) [b1].sms
	{ .crc = 0x05480C72, .rom = 0x800B, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Text Demo V1 by Super Majik Spiral Crew (PD) [b2].sms
	{ .crc = 0x1079CE44, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Text Demo V2 by Super Majik Spiral Crew (PD) [b1].sms
	{ .crc = 0xB20CC036, .rom = 0x8001, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Text Demo V2 by Super Majik Spiral Crew (PD) [b2].sms
	{ .crc = 0x8A098A14, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// SMSC Text Demo V2 by Super Majik Spiral Crew (PD).sms
	{ .crc = 0x357F000B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Snail Music Demo (PD).sms
	{ .crc = 0x304FE02B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sound Test by Nicolas Warren (PD).sms
	{ .crc = 0x1B1EFC66, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity Demo by Tetsujin (PD).sms
	{ .crc = 0x172474F3, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.0 by Proppy & Tet (PD).sms
	{ .crc = 0x33FDEA7A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.01 by Proppy & Tet (PD).sms
	{ .crc = 0x5ED9495B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.02 by Proppy & Tet (PD).sms
	{ .crc = 0x9039DB33, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.03 by Proppy & Tet (PD).sms
	{ .crc = 0x743E0E73, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.04 by Proppy & Tet (PD).sms
	{ .crc = 0xFBDE8C25, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.05 by Proppy & Tet (PD).sms
	{ .crc = 0x9A023763, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.06 by Proppy & Tet (PD).sms
	{ .crc = 0xE2D9560A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.07 by Proppy & Tet (PD).sms
	{ .crc = 0x6447D4E2, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.08 by Proppy & Tet (PD).sms
	{ .crc = 0x8ABB04FD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.09 by Proppy & Tet (PD).sms
	{ .crc = 0xEDA91464, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.10 by Proppy & Tet (PD).sms
	{ .crc = 0xBB8AA4D6, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.11 by Proppy & Tet (PD).sms
	{ .crc = 0xD57E9069, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.12 by Proppy & Tet (PD).sms
	{ .crc = 0x0FBC0916, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.13 by Proppy & Tet (PD).sms
	{ .crc = 0x6DE37953, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.14 by Proppy & Tet (PD).sms
	{ .crc = 0x17A8F966, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.15 by Proppy & Tet (PD).sms
	{ .crc = 0x08803E45, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.16 by Proppy & Tet (PD).sms
	{ .crc = 0x1D6D755E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.17 by Proppy & Tet (PD).sms
	{ .crc = 0xC5B73614, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.18 by Proppy & Tet (PD).sms
	{ .crc = 0x4A81A3EC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.19 by Proppy & Tet (PD).sms
	{ .crc = 0xA79AED1B, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.20 by Proppy & Tet (PD).sms
	{ .crc = 0x2BEB762D, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.21 by Proppy & Tet (PD).sms
	{ .crc = 0xC9D7A460, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.22 by Proppy & Tet (PD).sms
	{ .crc = 0x26ED3D83, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Oddity V.23 by Proppy & Tet (PD).sms
	{ .crc = 0xEA3D0D59, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sprite and Priority Test Program by Charles MacDonald (PD).sms
	{ .crc = 0xD6A8DD98, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sprite Multiplex Demo (03012005) by Charles MacDonald (PD).sms
	{ .crc = 0xFFCF390E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tototek Menu Boot - SG-1000 Mode by Chris Covell (PD).sms
	{ .crc = 0xD78C600D, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tototek Menu Boot - SMS Mode by Chris Covell (PD).sms
	{ .crc = 0xFE38661A, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultima III V2006-05-13prealpha (PD).sms
	{ .crc = 0x4E9CC98F, .rom = 0x7A01, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Unite Centrale de Huit Bits by Comic Bakery (PD).sms
	{ .crc = 0x046EBC1B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// V Counter Test by Charles MacDonald (PD).sms
	{ .crc = 0x6D0F1673, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vik 01 (PD).sms
	{ .crc = 0xAD300BD9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// ZEXALL V0.12SDSC by Maxim & Eric Quinn (PD).sms
	{ .crc = 0x05F471DE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// ZEXALL V0.12VDP by Maxim & Eric Quinn (PD).sms
	{ .crc = 0x38E4B272, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zoom Effect #1_1 by Charles MacDonald (PD).sms
	{ .crc = 0xA463DDFA, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zoom Effect #1_2 by Charles MacDonald (PD).sms
	{ .crc = 0xB97E110A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zoom Effect #1_3 by Charles MacDonald (PD).sms
	{ .crc = 0x980FDC4B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zoop's Birthday Demo (PD).sms
	{ .crc = 0x3396BF81, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Putt & Putter (E) [!].sms
	{ .crc = 0x357D4F78, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Putt & Putter (Prototype) [!].sms
	{ .crc = 0x8167CCC4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Q-bert (MV) [!].mv
	{ .crc = 0x77DB4704, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Double Target (J) [!].sms
	{ .crc = 0x52B83072, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Quartet (UE) [!].sms
	{ .crc = 0xE0F34FA6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Quartet (UE) [o1].sms
	{ .crc = 0x001DB6D7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R-Type (UE) [!].sms
	{ .crc = 0xBB54B6B0, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R-Type (UE) [T+Bra_Emuboarding].sms
	{ .crc = 0x7510BCE3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R-Type (UE) [T+Bra_Emuboarding][a1].sms
	{ .crc = 0xA3C80EE3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R.C. Grand Prix (UE) [!].sms
	{ .crc = 0x54316FEA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// R.C. Grand Prix (UE) [T+Bra_TMT].sms
	{ .crc = 0x6EDF4C12, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rainbow Islands (B) [!].sms
	{ .crc = 0x00EC173A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rainbow Islands (E) [!].sms
	{ .crc = 0xC172A22C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ashura (J) [!].sms
	{ .crc = 0xAE705699, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rambo - First Blood Part 2 (U) [!].sms
	{ .crc = 0xBBDA65F0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Secret Commando (E) [!].sms
	{ .crc = 0x00529114, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Secret Commando (E) [o1].sms
	{ .crc = 0x73CDF9DB, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rambo III (UE) [!].sms
	{ .crc = 0xDA5A7013, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rampage (UE) [!].sms
	{ .crc = 0x5F6E3412, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rampart (UE) [!].sms
	{ .crc = 0x426E5C8A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rastan (UE) [!].sms
	{ .crc = 0xC547EB1B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rastan (UE) [b1].sms
	{ .crc = 0x796C76B4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// American Baseball (JE) [!].sms
	{ .crc = 0x7B27202F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Reggie Jackson Baseball (U) [!].sms
	{ .crc = 0x6D94BB0E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ren & Stimpy - Quest for the Shaven Yak, The (B) [!].sms
	{ .crc = 0xF42E145C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Renegade (E) [!].sms
	{ .crc = 0x3BE7F641, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Renegade (E) [T+Bra_Emuboarding].sms
	{ .crc = 0xE3EC5B2D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rescue Mission (UE) [!].sms
	{ .crc = 0x79AC8E7F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Road Rash (E) [!].sms
	{ .crc = 0xB876FC74, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Road Rash (E) [b1].sms
	{ .crc = 0x6FE5BBC3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Robocop 3 (E) [!].sms
	{ .crc = 0x9F951756, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Robocop versus The Terminator (UE) [!].sms
	{ .crc = 0x8212B754, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rocky (UE) [!].sms
	{ .crc = 0x065E081F, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rocky (UE) [b1].sms
	{ .crc = 0x5D634D43, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Running Battle (E) [!].sms
	{ .crc = 0x1FDAE719, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sagaia (E) [!].sms
	{ .crc = 0x66388128, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sagaia (E) [b1].sms
	{ .crc = 0xCB5D369B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sangokushi 3 (K).sms
	{ .crc = 0x97D03541, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Satellite 7 (J) [!].sms
	{ .crc = 0x16249E19, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Satellite 7 (J) [p1][!].sms
	{ .crc = 0x87B9ECB8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Scramble Spirits (UE) [!].sms
	{ .crc = 0x9A8B28EC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Scramble Spirits (UE) [b1].sms
	{ .crc = 0x5DD789A2, .rom = 0x4009E, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega BASIC Level 2 (SC-3000).sc
	{ .crc = 0xF691F9C7, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega BASIC Level 3 V1 (SC-3000) [b1].sms
	{ .crc = 0x155FD01F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega BASIC Level 3 V1 (SC-3000).sc
	{ .crc = 0x5D9F11CA, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Chess (E) [!].sms
	{ .crc = 0xA8061AEF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Chess (E) [T+Bra_Altieres].sms
	{ .crc = 0x5B905D3F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Chess (E) [T+Spa].sms
	{ .crc = 0x2ECFE5F2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Music Editor (SC-3000) [!].sc
	{ .crc = 0x2EC28526, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Music Editor (SC-3000) [a1].sc
	{ .crc = 0x622010E1, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega Music Editor (SC-3000) [b1].sms
	{ .crc = 0xB67EA1C4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sega World Tournament Golf (E) [!].sms
	{ .crc = 0x296879DD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sensible Soccer (E) [!].sms
	{ .crc = 0xF8176918, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shadow Dancer (E) [!].sms
	{ .crc = 0x3793C01A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shadow Dancer (E) [b1].sms
	{ .crc = 0x4135AE1D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shadow of the Beast (E) [!].sms
	{ .crc = 0x1575581D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shanghai (UE) [!].sms
	{ .crc = 0xAAB67EC3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shanghai (UE) [o1].sms
	{ .crc = 0x8D3D6BFF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shinobi (J) [!].sms
	{ .crc = 0xE1FFF1BB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shinobi (UE) [!].sms
	{ .crc = 0x11FDDFB2, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shinobi (UE) [b1].sms
	{ .crc = 0x483BD1C8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shinobi (UE) [b2].sms
	{ .crc = 0x4BFD45F0, .rom = 0x20D00, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shinobi (UE) [b3].sms
	{ .crc = 0xB2F41AD6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Shooting Gallery (UE) [!].sms
	{ .crc = 0x4B051022, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Simpsons, The - Bart vs. the Space Mutants (E) [!].sms
	{ .crc = 0xD1CC08EE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Simpsons, The - Bart vs. the World (E) [!].sms
	{ .crc = 0xF6B2370A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sindbad Mystery (SG-1000) [b1].sms
	{ .crc = 0x54D5BEC3, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sitio do Picapau Amarelo (B) [!].sms
	{ .crc = 0xABDF3923, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Slap Shot (E) [!].sms
	{ .crc = 0xC93BD0E9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Slap Shot (U) (V1.0) [!].sms
	{ .crc = 0xD33B296A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Slap Shot (U) (V1.0) [b1].sms
	{ .crc = 0x0473142F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Slap Shot (U) (V1.0) [b2].sms
	{ .crc = 0x074F0893, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Slap Shot (U) (V1.1) [!].sms
	{ .crc = 0x702C3E98, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Smurfs Travel the World, The (E) (M4) [!].sms
	{ .crc = 0x97E5BB7D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Smurfs, The (E) [!].sms
	{ .crc = 0x3E63768A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Solomon no Kagi - Oujo Rihita no Namida (J) [!].sms
	{ .crc = 0x11645549, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic Blast (B) [!].sms
	{ .crc = 0x96B3F29E, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic Chaos (E) [!].sms
	{ .crc = 0xAEDF3BDF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic Chaos (E) [b1].sms
	{ .crc = 0xFB14F1CF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic Spinball (E) [!].sms
	{ .crc = 0x11C1BC8A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic 2 Rebirth (Sonic 2 Hack).sms
	{ .crc = 0x54F91CBA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic The Hedgehog 2 (UE) (V1.0) [!].sms
	{ .crc = 0x5B3B922C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic The Hedgehog 2 (UE) (V2.2).sms
	{ .crc = 0xD6F2BFCA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic The Hedgehog (UE) [!].sms
	{ .crc = 0xB519E833, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic The Hedgehog (UE) [T+Fre_Terminus].sms
	{ .crc = 0x48806FF7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier 3D (J) [!].sms
	{ .crc = 0x156948F9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier 3D (UE) [!].sms
	{ .crc = 0x6BD5C2BF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier (J) [!].sms
	{ .crc = 0xBEDDF80E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier (J) [o1].sms
	{ .crc = 0xFCD537D6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier (UE) [!].sms
	{ .crc = 0xD78F44AE, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Space Harrier (UE) [b1].sms
	{ .crc = 0x47BD2006, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spacegun (E) [!].sms
	{ .crc = 0xA908CFF5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Special Criminal Investigation (E) [!].sms
	{ .crc = 0xFA8E4CA0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Special Criminal Investigation (Prototype) [!].sms
	{ .crc = 0x1B7D2A20, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Speedball 2 (E) [!].sms
	{ .crc = 0x0C7366A0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Speedball (E) (Mirrorsoft) [!].sms
	{ .crc = 0xA57CAD18, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Speedball (E) (Virgin) [!].sms
	{ .crc = 0x5CCC1A65, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kujaku Ou (J) [!].sms
	{ .crc = 0xD11D32E4, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Kujaku Ou (J) [b1].sms
	{ .crc = 0x68108314, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spellcaster (UE) [!].sms
	{ .crc = 0x4752CAE7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spellcaster (UE) [h1].sms
	{ .crc = 0xDA375ABD, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spellcaster (UE) [T+Fre].sms
	{ .crc = 0x9B1F566E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spellcaster (UE) [T+Fre][a1].sms
	{ .crc = 0xA0587800, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spider-Man - Return of the Sinister Six (E) [!].sms
	{ .crc = 0xEBE45388, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spider-Man vs. the Kingpin (UE) [!].sms
	{ .crc = 0x908FF25C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sports Pad Football (U) [!].sms
	{ .crc = 0xE42E4998, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spy vs. Spy (J) [!].sms
	{ .crc = 0xD41B9A08, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spy vs. Spy (J) [h1].sms
	{ .crc = 0xA71BC542, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spy vs. Spy (U) (Display-Unit Cart) [!].sms
	{ .crc = 0xB87E1B2B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Spy vs. Spy (UE) [!].sms
	{ .crc = 0x78D7FAAB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Star Wars (UE) [!].sms
	{ .crc = 0xD4B8F66D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Street Fighter 2 (B) [!].sms
	{ .crc = 0x0F8287EC, .rom = 0xC8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Street Fighter 2 (B) [b1].sms
	{ .crc = 0x87AABDFE, .rom = 0xC8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Street Fighter 2 (B) [b2].sms
	{ .crc = 0x599901BB, .rom = 0xC8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Streets of Rage 2 (E) [!].sms
	{ .crc = 0x04E9C089, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Streets of Rage (E) [a1].sms
	{ .crc = 0x4AB3790F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Streets of Rage (E) [b1].sms
	{ .crc = 0x6F2CF48A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Streets of Rage (E) [T+Fre_1.0].sms
	{ .crc = 0xC95E0CC2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Streets of Rage (E) [T-Fre].sms
	{ .crc = 0xECC18147, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider II (UE) [!].sms
	{ .crc = 0xB8F0915A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [!].sms
	{ .crc = 0x131BDB98, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [b1].sms
	{ .crc = 0x5B731A99, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [b1][T+Bra_CBT].sms
	{ .crc = 0x54A82E50, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [T+Bra_CBT].sms
	{ .crc = 0x53C2FEB2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [T+Bra_Emunow].sms
	{ .crc = 0xCB6F3D1A, .rom = 0x80200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Strider (UE) [T+Bra_Emunow][a1].sms
	{ .crc = 0x471CDB51, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Submarine Attack (UE) [!].sms
	{ .crc = 0xD8F2F1B9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sukeban Deka 2 (J) [!].sms
	{ .crc = 0xB13DF647, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Summer Games (E) [!].sms
	{ .crc = 0x8DA5C93F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Basketball (Demo) [!].sms
	{ .crc = 0x0DBF3B4A, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Kick Off (E) [!].sms
	{ .crc = 0x406AA0C2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Kick Off (E) [b1].sms
	{ .crc = 0x29768BEA, .rom = 0x40111, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Monaco GP (UE) (V1.0) [!].sms
	{ .crc = 0x3EF12BAA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Monaco GP (UE) (V1.1) [!].sms
	{ .crc = 0x55BF81A0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Off-Road (E) [!].sms
	{ .crc = 0x54F68C2A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Racing (J) [!].sms
	{ .crc = 0x7E0EF8CB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Smash TV (E) [!].sms
	{ .crc = 0xE0B1AFF8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Space Invaders (E) [!].sms
	{ .crc = 0x1D6244EE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Tennis (J).sms
	{ .crc = 0x95CBF3DD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Tennis (UE) [!].sms
	{ .crc = 0xF927F41C, .rom = 0x8200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Superman (E) [!].sms
	{ .crc = 0x6F9AC98F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Superman (E) [T+Bra_guto].sms
	{ .crc = 0x76B86E59, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taito Chase H.Q. (J) [!].sms
	{ .crc = 0x85CFC9C9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taito Chase H.Q. (J) [o1].sms
	{ .crc = 0xEEEAAEE5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tanoshii Sansuu (SC-3000) [!].sc
	{ .crc = 0x6A96978D, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taz in Escape From Mars (B) [!].sms
	{ .crc = 0x11CE074C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taz-Mania (E) [!].sms
	{ .crc = 0x7CC3E837, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taz-Mania (Prototype) [!].sms
	{ .crc = 0x1B312E04, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Taz-Mania (Prototype) [T+Bra1.0_EmuBrazil][p1][!].sms
	{ .crc = 0xC63E7B0A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tecmo World Cup '92 (Prototype for '93) [!].sms
	{ .crc = 0x96E75F48, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tecmo World Cup '93 (E) [!].sms
	{ .crc = 0x5A1C3DDE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Felipe em Acao (B).sms
	{ .crc = 0xCCB2CAB4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Geraldinho (B).sms
	{ .crc = 0x956C416B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Mario Bros (Teddy Boy Hack).sms
	{ .crc = 0x76BBD33E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Zelda (Teddy Boy Hack) [a1].sms
	{ .crc = 0xB936F761, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Zelda (Teddy Boy Hack).sms
	{ .crc = 0xE29C4016, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pokemon Versao Amarela (Teddy Boy Hack).sms
	{ .crc = 0x6C476AA9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pokemon Versao Azul (Teddy Boy Hack).sms
	{ .crc = 0xD0E919BD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pokemon Versao Secreta (Teddy Boy Hack).sms
	{ .crc = 0x647D8EDC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Pokemon Versao Vermelha (Teddy Boy Hack).sms
	{ .crc = 0x3F78E5C8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo 2 em - Mate O Leonardo (Teddy Boy Hack).sms
	{ .crc = 0x9D79ED79, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo 2 em - Mate O Papai Noel - Versao de Natal (Teddy Boy Hack).sms
	{ .crc = 0xC7F3DA07, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic-Boy 3 - Eggman's Room (Teddy Boy Hack).sms
	{ .crc = 0xF8C3D4F4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Teddy Boy (UE) [!].sms
	{ .crc = 0x2728FAA3, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Teddy Boy Blues (J) (EP-MyCard).sms
	{ .crc = 0xD7508041, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Teddy Boy Blues (J) [!].sms
	{ .crc = 0x316727DD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Teddy Boy Blues (J) [p1][!].sms
	{ .crc = 0x9DFA67EE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Teddy Boy e Geraldinho (B) (Geraldinho Hack).sms
	{ .crc = 0x8FC6FEF9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tennis Ace (E) [!].sms
	{ .crc = 0x1A390B93, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tensai Bakabon (J) [!].sms
	{ .crc = 0x8132AB2C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Terminator 2 - Judgment Day (E) [!].sms
	{ .crc = 0xAC56104F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Terminator 2 - The Arcade Game (E) [!].sms
	{ .crc = 0x93CA8152, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Terminator, The (E) [!].sms
	{ .crc = 0xEDC5C012, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Three Dragon Story, The (K).sms
	{ .crc = 0x8640E7E8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Thunder Blade (J) [!].sms
	{ .crc = 0xC0CE19B1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Thunder Blade (UE) [!].sms
	{ .crc = 0xB3007DB7, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Time Soldiers (UE) [!].sms
	{ .crc = 0x4C2F6742, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Time Soldiers (UE) [o1].sms
	{ .crc = 0xDC2A88CB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Time Soldiers (UE) [T+Bra_CBT].sms
	{ .crc = 0xFF0F9323, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Time Soldiers (UE) [T+Bra_Emunow].sms
	{ .crc = 0xED9A19B6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tom and Jerry (Prototype) [!].sms
	{ .crc = 0x0C2FC2DE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tom and Jerry - The Movie (E) [b1].sms
	{ .crc = 0xA2FF09B9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tom and Jerry - The Movie (E) [b2].sms
	{ .crc = 0xB003F4EB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Tom and Jerry - The Movie (E).sms
	{ .crc = 0xBF7B7285, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Toto World 3 (K) [!].sms
	{ .crc = 0x4F8D75EC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Flash (J) [!].sms
	{ .crc = 0xC795182D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Astro Flash (J) [p1][!].sms
	{ .crc = 0x0E21E6CF, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trans-Bot (UE) [!].sms
	{ .crc = 0x4BC42857, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trans-Bot (UE) [o1].sms
	{ .crc = 0xD62F520A, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trans-Bot (UE) [o2].sms
	{ .crc = 0x69362BF1, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trans-Bot (UE) [o3].sms
	{ .crc = 0xBD6F2C92, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trap Shooting - Marksman Shooting - Safari Hunt (UE) [!].sms
	{ .crc = 0xE8215C2E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Trivial Pursuit (E) [!].sms
	{ .crc = 0xE5374022, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultima IV - Quest of the Avatar (E) [!].sms
	{ .crc = 0xB52D60C8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultima IV - Quest of the Avatar (E) [T+Ger].sms
	{ .crc = 0x19D64680, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultima IV - Quest of the Avatar (Prototype) [!].sms
	{ .crc = 0xDE9F8517, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultimate Soccer (E) [!].sms
	{ .crc = 0x15668CA4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ultimate Soccer (E) [b1].sms
	{ .crc = 0x7840D45C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vigilante (UE) [!].sms
	{ .crc = 0xDFB0B161, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Vigilante (UE) [T+Fre_Terminus].sms
	{ .crc = 0xA986542B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Virtua Fighter Animation (J) [!].sms
	{ .crc = 0x57F1545B, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// WWF Wrestlemania Steel Cage Challenge (U) [!].sms
	{ .crc = 0x2DB21448, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// American Pro Football (E) [!].sms
	{ .crc = 0x3727D8B2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Walter Payton Football (U) [!].sms
	{ .crc = 0x3D55759B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Walter Payton Football (U) [o1].sms
	{ .crc = 0x73B9D9BB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wanted (UE) [!].sms
	{ .crc = 0x5359762D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Where in the World is Carmen Sandiego (B).sms
	{ .crc = 0x88AA8CA6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Where in the World is Carmen Sandiego (U) [!].sms
	{ .crc = 0x428B1E7C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wimbledon II (E) [!].sms
	{ .crc = 0x7F3AFE58, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wimbledon (E) [!].sms
	{ .crc = 0x912D92AF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Winter Olympics '94 (B) (M8) [!].sms
	{ .crc = 0x2FEC2B4A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Winter Olympics '94 (UE) (M8) [!].sms
	{ .crc = 0xA20290B6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wolf Child (UE) [!].sms
	{ .crc = 0x1F8EFA1D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Guerreiro Lobo V2.0 (Wonder Boy III Hack).sms
	{ .crc = 0xD3C52B09, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic-Boy 2 - Eggman's Trap (Wonder Boy III Hack).sms
	{ .crc = 0x6259AE57, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Turma da Monica em - O Resgate (B) [!].sms
	{ .crc = 0x22CCA9BB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy III - The Dragon's Trap (UE) [!].sms
	{ .crc = 0x679E1676, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy III - The Dragon's Trap (UE) [o1].sms
	{ .crc = 0xDA7762AF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy III - The Dragon's Trap (UE) [T+Fre].sms
	{ .crc = 0x79EC738B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy III - The Dragon's Trap (UE) [T+Swe.7b_Metalhead].sms
	{ .crc = 0x42012B7D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy III D.C. (Wonder Boy III Hack).sms
	{ .crc = 0xCA500A43, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Magali no Castelo do Dragao (Monica no Castelo Hack).sms
	{ .crc = 0x55C88828, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Mega Man (Wonder Boy in ML Hack) [o1].sms
	{ .crc = 0xE15BA28B, .rom = 0x40040, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Master Mega Man (Wonder Boy in ML Hack).sms
	{ .crc = 0xD164FDE7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monica no Castelo do Dragao (B) [!].sms
	{ .crc = 0x01D67C0B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monica no Castelo do Dragao (B) [b1].sms
	{ .crc = 0xFAC09AF4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic-Boy in Monster Land (Wonder Boy in ML Hack).sms
	{ .crc = 0x6A16DCE4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Wonder Boy - Monster World (J) (V1.0) [!].sms
	{ .crc = 0xB1DA6A30, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Turma da Monica 3 v01 (Monster World III Hack).sms
	{ .crc = 0x458390E7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster Land (UE) (V1.1) [!].sms
	{ .crc = 0x8CBEF0C1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster Land (UE) (V1.1) [h1].sms
	{ .crc = 0x7522CF0A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster Land (UE) (V1.1) [T+Fre_Crispysix].sms
	{ .crc = 0x43933407, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Monica 3 (Wonder Boy in MW Hack).sms
	{ .crc = 0x693C63D5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sonic Boy 4 - Monster World (Wonder Boy in MW Hack).sms
	{ .crc = 0x11637350, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Treinamento Do Guerreiro Lobo (Wonder Boy in MW Hack) [a1].sms
	{ .crc = 0x86F60EF6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Treinamento Do Guerreiro Lobo (Wonder Boy in MW Hack).sms
	{ .crc = 0x8CC9B324, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster World (E) [!].sms
	{ .crc = 0x7D7CE80B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster World (E) [T+Fre_Microtrads].sms
	{ .crc = 0x5C87B0AE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster World (Prototype) [!].sms
	{ .crc = 0x81BFF9BB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy in Monster World (Prototype) [b1].sms
	{ .crc = 0xFA8B5F3D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo O Resgate (Wonder Boy Hack) [a1].sms
	{ .crc = 0x947DB11A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo O Resgate (Wonder Boy Hack) [a1][o1].sms
	{ .crc = 0x628BA710, .rom = 0x21B10, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo O Resgate (Wonder Boy Hack) [o1].sms
	{ .crc = 0x0DE4BB6B, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Rodrigo O Resgate (Wonder Boy Hack).sms
	{ .crc = 0x1EE6E034, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Super Wonder Boy (J) [!].sms
	{ .crc = 0xE2FCB6F3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy (UE) [!].sms
	{ .crc = 0x73705C02, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Wonder Boy (UE) [o1].sms
	{ .crc = 0xF1EE83CF, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Woody Pop (J) [!].sms
	{ .crc = 0x315917D4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Class Leaderboard (E) [!].sms
	{ .crc = 0xC9A449B7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Cup '94 (UE) [!].sms
	{ .crc = 0xA6BF8F9E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Cup Italia '90 (E) [!].sms
	{ .crc = 0x6E1AD6FD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Cup Italia '90 (E) [b1].sms
	{ .crc = 0x1ED55767, .rom = 0x20200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Cup Italia '90 (E) [T+Bra][p1][!].sms
	{ .crc = 0x2377FA48, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Games (E) [!].sms
	{ .crc = 0xA2A60BC8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Games (Prototype) [!].sms
	{ .crc = 0x914D3FC4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Circuit, The (J) [!].sms
	{ .crc = 0x8FB75994, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Grand Prix (E) [!].sms
	{ .crc = 0x4AAAD0D6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Grand Prix (E) [o1].sms
	{ .crc = 0xFF6E23B0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Grand Prix (U) [!].sms
	{ .crc = 0x7B369892, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Sports Pad Soccer (J) [!].sms
	{ .crc = 0x41C948BF, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Soccer (E) [!].sms
	{ .crc = 0x72112B75, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Soccer (E) [o1].sms
	{ .crc = 0xBAE431A4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// World Soccer (UE) [b1].sms
	{ .crc = 0xAB260100, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// X-Men - Mojo World (B) [!].sms
	{ .crc = 0x3E1387F6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Xenon 2 (E) (Image Works) [!].sms
	{ .crc = 0x5C205EE1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Xenon 2 (E) (Virgin) [!].sms
	{ .crc = 0xEC726C0D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Xenon 2 (E) (Virgin) [b1].sms
	{ .crc = 0xF13E118D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ys (J).sms
	{ .crc = 0x32759751, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ys - The Vanished Omens (UE) [!].sms
	{ .crc = 0xB33E2827, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Ys - The Vanished Omens (UE) [T+Fre_Crispysix].sms
	{ .crc = 0xAFD29460, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zaxxon 3D (Prototype).sms
	{ .crc = 0xBBA74147, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zaxxon 3D (UE) [!].sms
	{ .crc = 0xA3EF13CB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion II - The Tri Formation (UE) [!].sms
	{ .crc = 0x2F2E3BC9, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion II - The Tri Formation (UE) [b1].sms
	{ .crc = 0xF0F81241, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion II - The Tri Formation (UE) [o1].sms
	{ .crc = 0x2EBACAFC, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Akai Koudan Zillion (J) (V1.0) [!].sms
	{ .crc = 0x60C19645, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (UE) (V1.1) [!].sms
	{ .crc = 0x5718762C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (UE) (V1.1) [b1][o1].sms
	{ .crc = 0xEF88FCAA, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (UE) (V1.1) [o1].sms
	{ .crc = 0xB12CC8EC, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (UE) (V1.1) [T+Bra_CBT].sms
	{ .crc = 0xDABC7C0E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (UE) (V1.1) [T+Bra_CBT][o1].sms
	{ .crc = 0x3DF9BC7D, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zillion (US) (V1.1) [T+Fre_Terminus].sms
	{ .crc = 0xE51F3D1B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
	// Zool (E) [!].sms
	{ .crc = 0x9D9D0A5F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_SMS },
/* -----------------------------------------------------*/
// GAMEGEAR
/* -----------------------------------------------------*/
    // 5 in 1 Funpak (U) [!].gg
	{ .crc = 0xF85A8CE8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aah! Harimanada (J).gg
	{ .crc = 0x1D17D2A0, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Addams Family, The (U) [!].gg
	{ .crc = 0x1D01F999, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Adventures of Batman & Robin, The (U) [!].gg
	{ .crc = 0xBB4F23FF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aerial Assault (JU) (V1.0) [b1].gg
	{ .crc = 0x899515D0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aerial Assault (JU) (V1.0).gg
	{ .crc = 0x3E549B7A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aerial Assault (JU) (V1.1) [!].gg
	{ .crc = 0x04FE6DDE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aladdin (J) [b1].gg
	{ .crc = 0x488CDA3C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aladdin (J).gg
	{ .crc = 0x770E95E1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aladdin (U) [!].gg
	{ .crc = 0x7A41C1DC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aladdin (U) [T+Bra_ERTrans].gg
	{ .crc = 0x840F457E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Aladdin (U) [T-Bra_ERTrans].gg
	{ .crc = 0x691BF489, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Alien 3 (U) [!].gg
	{ .crc = 0x11A68C08, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Alien Syndrome (J).gg
	{ .crc = 0xFFE4ED47, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Andre Agassi Tennis (U) [!].gg
	{ .crc = 0x46C9FA3E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Arcade Classics (U) [!].gg
	{ .crc = 0x3DECA813, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Arch Rivals (U) [!].gg
	{ .crc = 0xF0204191, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Arena - Maze of Death (U) [!].gg
	{ .crc = 0x7CB3FACF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ariel - The Little Mermaid (U) [!].gg
	{ .crc = 0x97E3A18C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Arliel - Crystal Densetsu (J).gg
	{ .crc = 0x35FA3F68, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Asterix and the Great Rescue (E) (M5) [!].gg
	{ .crc = 0x328C5CC8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Asterix and the Great Rescue (U) [!].gg
	{ .crc = 0x78208B40, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (J) (V1.5) [b1].gg
	{ .crc = 0x1023FF26, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (J) (V1.5).gg
	{ .crc = 0xDFCF555F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (U) (V2.4) [!].gg
	{ .crc = 0x663BCF8A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (U) (V2.4) [T+Fre].gg
	{ .crc = 0x1A2D6E19, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (U) (V2.4) [T+Fre][a1].gg
	{ .crc = 0x3327FB6E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ax Battler - A Legend of Golden Axe (U) (V2.4) [T+Spa100_pkt].gg
	{ .crc = 0x71D34B14, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ayrton Senna's Super Monaco GP II (J) [b1].gg
	{ .crc = 0x346E1CF1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ayrton Senna's Super Monaco GP II (J).gg
	{ .crc = 0x661FAEFF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ayrton Senna's Super Monaco GP II (U) [!].gg
	{ .crc = 0x1D1B1DD3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Baku Baku Animal - Sekai Shiikugakari Senshu-ken (J) [!].gg
	{ .crc = 0x10AC9374, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Baku Baku Animal (U).gg
	{ .crc = 0x8D8BFDC4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batman Forever (U) [!].gg
	{ .crc = 0x618B19E2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batman Returns (U) [!].gg
	{ .crc = 0x7AC4A3CA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batman Returns (U) [b1].gg
	{ .crc = 0x65E60DFE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batman Returns (U) [b1][t1].gg
	{ .crc = 0x79DB0265, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batman Returns (U) [t1].gg
	{ .crc = 0x66F9AC51, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [!].gg
	{ .crc = 0xFDDD8CD9, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [b1].gg
	{ .crc = 0x163D81C1, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [b1][o1].gg
	{ .crc = 0xD8342782, .rom = 0x60000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [o1].gg
	{ .crc = 0x53F37C4C, .rom = 0x60000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [o2].gg
	{ .crc = 0xE9987511, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battleship (U) [o3].gg
	{ .crc = 0x3BACAAEC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battletoads (U) [!].gg
	{ .crc = 0x817CC0CA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Battletoads (U) [a1].gg
	{ .crc = 0xCB3CD075, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Beavis and Butt-head (U) [!].gg
	{ .crc = 0xA6BF865E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Beavis and Butt-head (U) [a1].gg
	{ .crc = 0x3858F14F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Berenstain Bears', The - Camping Adventure (U) [!].gg
	{ .crc = 0xA4BB9FFB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Berlin no Kabe (J) (GG2SMS V1.0 Hack).gg
	{ .crc = 0xB672AE7A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Berlin no Kabe (J) (GG2SMS V1.1 Hack).gg
	{ .crc = 0xDC477A0D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Berlin no Kabe (J).gg
	{ .crc = 0x325B1797, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bishoujo Senshi Sailor Moon S (J) [b1].gg
	{ .crc = 0xEDE33592, .rom = 0x800CB, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bishoujo Senshi Sailor Moon S (J) [b2].gg
	{ .crc = 0x857C6D83, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bishoujo Senshi Sailor Moon S (J) [b3].gg
	{ .crc = 0xF8B66406, .rom = 0x386BE, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bishoujo Senshi Sailor Moon S (J).gg
	{ .crc = 0xFE7374D2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bonkers Wax Up! (U) [!].gg
	{ .crc = 0xBFCEBA5F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bram Stoker's Dracula (U) [!].gg
	{ .crc = 0x69EBE5FA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bram Stoker's Dracula (U) [b1].gg
	{ .crc = 0xD966EC47, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bubble Bobble (U) [!].gg
	{ .crc = 0xFBA338C5, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bugs Bunny in Double Trouble (U) [!].gg
	{ .crc = 0x5C34D9CD, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Bust-A-Move (U) [!].gg
	{ .crc = 0xC90F29EF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Buster Ball (J).gg
	{ .crc = 0x7CB079D0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Buster Fight (J).gg
	{ .crc = 0xA72A1753, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batter Up (U) [!].gg
	{ .crc = 0x16448209, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batter Up (U) [b1].gg
	{ .crc = 0x945E0E4C, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Batter Up (U) [b2].gg
	{ .crc = 0xC10946CB, .rom = 0x14000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Caesar's Palace (U) [!].gg
	{ .crc = 0xC53B60B8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Captain America and the Avengers (U) [!].gg
	{ .crc = 0x5675DFDD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Car Licence (J).gg
	{ .crc = 0xF6A697F8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Casino Funpak (U) [!].gg
	{ .crc = 0x2B732056, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Castle of Illusion Starring Mickey Mouse (J) [S][!].gg
	{ .crc = 0x9942B69B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Castle of Illusion Starring Mickey Mouse (J) [S][h1].gg
	{ .crc = 0x5877B10D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Castle of Illusion Starring Mickey Mouse (U) [S][!].gg
	{ .crc = 0x59840FD6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chakan (U) [!].gg
	{ .crc = 0xDFC7ADC8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chaos 89 (PD).gg
	{ .crc = 0x820CEB2C, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chase H.Q. (J) [S][!].gg
	{ .crc = 0x44FBE8F6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chase H.Q. (J) [S][b1].gg
	{ .crc = 0x7BB81E3D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chase H.Q. (J) [S][o1].gg
	{ .crc = 0x18086B70, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chase H.Q. (J) [S][o1][b1].gg
	{ .crc = 0x3B627808, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chessmaster, The (U) [!].gg
	{ .crc = 0xDA811BA6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chicago Syndicate (U) [!].gg
	{ .crc = 0x6B0FCEC3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Choplifter III (U) [!].gg
	{ .crc = 0xC2E8A692, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chuck Rock II - Son of Chuck (E) [!].gg
	{ .crc = 0x656708BF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chuck Rock II - Son of Chuck (U) [!].gg
	{ .crc = 0x3FCC28C9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chuck Rock (U) [!].gg
	{ .crc = 0x191B1ED8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chuck Rock (U) [t1].gg
	{ .crc = 0xD495C31D, .rom = 0x44000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Chuck Rock (U) [t1][o1].gg
	{ .crc = 0xB1D01697, .rom = 0x44001, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Cliffhanger (U) [!].gg
	{ .crc = 0xBF75B153, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Clutch Hitter (U) [!].gg
	{ .crc = 0xD228A467, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Coca Cola Kid (J) [!].gg
	{ .crc = 0xC7598B81, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Columns (J).gg
	{ .crc = 0xF3CA6676, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Columns (U) [!].gg
	{ .crc = 0x83FA26D9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Cool Spot (U) [!].gg
	{ .crc = 0x2C758FC8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Cosmic Spacehead (E) (M4) [!].gg
	{ .crc = 0x6CAA625B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Crayon Shin-Chan - Taiketsu! Tankam Panic!! (J).gg
	{ .crc = 0x03D28EAB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Crystal Warriors (U) [!].gg
	{ .crc = 0x529C864E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Crystal Warriors (U) [T+Fre.99_Asmodeath].gg
	{ .crc = 0x3473200A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Crystal Warriors (U) [T-Fre].gg
	{ .crc = 0xF3925382, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Cutthroat Island (U) [!].gg
	{ .crc = 0x6A24E47E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Daffy Duck in Hollywood (E) (M5) [!].gg
	{ .crc = 0xAAE268FC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Daffy Duck in Hollywood (E) (M5) [b1].gg
	{ .crc = 0x16ABE1EE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Daffy Duck in Hollywood (E) (M5) [b2].gg
	{ .crc = 0xCF41EB97, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Deep Duck Trouble Starring Donald Duck (J) [t1].gg
	{ .crc = 0x50A2D08F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Deep Duck Trouble Starring Donald Duck (J).gg
	{ .crc = 0x4457E7C0, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Deep Duck Trouble Starring Donald Duck (U) [!].gg
	{ .crc = 0x5A136D7E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Defenders of Oasis (U) [!].gg
	{ .crc = 0xE2791CC1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Desert Speedtrap - Starring Road Runner and Wile E. Coyote (E) (M5) [!].gg
	{ .crc = 0xEC808026, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Desert Speedtrap - Starring Road Runner and Wile E. Coyote (U) [!].gg
	{ .crc = 0xC2E111AC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Desert Strike - Return to the Gulf (U) [!].gg
	{ .crc = 0xF6C400DA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Desert Strike - Return to the Gulf (U) [b1].gg
	{ .crc = 0xCD46F641, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Devilish (J).gg
	{ .crc = 0x25DB174F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Devilish (U) [!].gg
	{ .crc = 0xC01293B0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Doraemon - Waku Waku Pocket Paradise (J) [!].gg
	{ .crc = 0x733292A6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Double Dragon (Prototype).gg
	{ .crc = 0x331904C0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Double Dragon (U) [!].gg
	{ .crc = 0x1307A290, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dr. Robotnik's Mean Bean Machine (U) [!].gg
	{ .crc = 0x3C2D4F48, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dragon - The Bruce Lee Story (E) [!].gg
	{ .crc = 0x19E1CF2B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dragon - The Bruce Lee Story (U) [!].gg
	{ .crc = 0x17F588E8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dragon Crystal (J).gg
	{ .crc = 0x89F12E1E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dragon Crystal (U) [!].gg
	{ .crc = 0x0EF2ED93, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Drop Zone (U) [!].gg
	{ .crc = 0x152F0DCC, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Dunk Kid's (J).gg
	{ .crc = 0x77ED48F5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dynamite Headdy (J) [b1].gg
	{ .crc = 0xFDC8FC18, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dynamite Headdy (J).gg
	{ .crc = 0xF6AF4B6B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dynamite Headdy (U) [b1].gg
	{ .crc = 0x73DD5D0C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Dynamite Headdy (U).gg
	{ .crc = 0x610FF95C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Earthworm Jim (U) [!].gg
	{ .crc = 0x5D3F23A9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Earthworm Jim (U) [t1].gg
	{ .crc = 0x5512780D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ecco - The Tides of Time (U) [!].gg
	{ .crc = 0xE2F3B203, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ecco II - The Tides of Time (J) [!].gg
	{ .crc = 0xBA9CEF4F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ecco the Dolphin (U) [!].gg
	{ .crc = 0x866C7113, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ecco the Dolphin (U) [b1].gg
	{ .crc = 0x2B60873E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ecco the Dolphin (U) [b2].gg
	{ .crc = 0x982E7132, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ernie Els Golf (E).gg
	{ .crc = 0x5E53C7F7, .rom = 0x40000, .ram = 0x2000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Eternal Legend (J) [!].gg
	{ .crc = 0x04302BBD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Evander Holyfield's 'Real Deal' Boxing (U) [!].gg
	{ .crc = 0x36AAF536, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Excellent Dizzy Collection, The (E) [S][!].gg
	{ .crc = 0xAA140C9C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Excellent Dizzy Collection, The (Prototype) [S][!].gg
	{ .crc = 0x8813514B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// F-1 (E) [!].gg
	{ .crc = 0xD0A93E00, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// F-15 Strike Eagle (U) [!].gg
	{ .crc = 0x8BDB0806, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// FIFA International Soccer (J).gg
	{ .crc = 0x78159325, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// FIFA International Soccer (UE) (M4) [!].gg
	{ .crc = 0xE632A3A2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// FIFA Soccer 96 (UE) (M4) [!].gg
	{ .crc = 0xC379DE95, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Faceball 2000 (J) [!].gg
	{ .crc = 0xAAF6F87D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Factory Panic (U) [!].gg
	{ .crc = 0x59E3BE92, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fantastic Dizzy (U) [S][!].gg
	{ .crc = 0xC888222B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Fantasy Zone Gear (JU) [!].gg
	{ .crc = 0xD69097E8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fantasy Zone Gear (JU) [a1].gg
	{ .crc = 0x365B92CF, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fatal Fury Special (U) [!].gg
	{ .crc = 0x449787E2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fatal Fury Special (U) [b1].gg
	{ .crc = 0x29B1DF1A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fatal Fury Special (U) [b2].gg
	{ .crc = 0x213F53EA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fire Track by Ben Ryves (PD).gg
	{ .crc = 0xD6D0A92B, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Foreman for Real (U) [!].gg
	{ .crc = 0xD46D5685, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frank Thomas Big Hurt Baseball (U) [!].gg
	{ .crc = 0xC443C35C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fred Couples Golf (J).gg
	{ .crc = 0xB1196CD7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Fred Couples Golf (U) [!].gg
	{ .crc = 0x46F40B9F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) (GG2SMS V0.91 Hack) [o1].gg
	{ .crc = 0x9ACA67FB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) (GG2SMS V0.91 Hack).gg
	{ .crc = 0xF5D98348, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) (GG2SMS V0.92 Hack) [o1].gg
	{ .crc = 0x1C6044B0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) (GG2SMS V0.92 Hack).gg
	{ .crc = 0x444ABB05, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) [!].gg
	{ .crc = 0x02BBF994, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogger (Prototype) [o1].gg
	{ .crc = 0xDA724710, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Frogs by Charles Doty (PD).gg
	{ .crc = 0xDBDB3A2E, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// From TV Animation - Slam Dunk - Shouri heno Starting 5 (J) [!].gg
	{ .crc = 0x751DAD4C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// G-LOC Air Battle (J) (V1.0) [!].gg
	{ .crc = 0x2333F615, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// G-LOC Air Battle (J) (V1.1).gg
	{ .crc = 0x33237F50, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// G-LOC Air Battle (U) [!].gg
	{ .crc = 0x18DE59ED, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Aleste (J).gg
	{ .crc = 0x1B80A75B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Demo by Charles MacDonald (PD).gg
	{ .crc = 0xACF7E6A2, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Doraemon - Nora no Suke no Yabou (J).gg
	{ .crc = 0x9A8B2C16, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Hi-Res Graphics Demo by Charles McDonald (PD).gg
	{ .crc = 0x88829996, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Hicolor Demo by Chris Covell (PD).gg
	{ .crc = 0xE2E209C4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Hicolor Demo v1.05 by Chris Covell (PD).gg
	{ .crc = 0x1882C8D7, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Nibbles by Martin Konrad V1 (PD).gg
	{ .crc = 0xE0B5215A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Nibbles by Martin Konrad V2 (PD).gg
	{ .crc = 0x2E6C15AB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Nibbles by Martin Konrad V3 (PD).gg
	{ .crc = 0x8CA15176, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Nibbles by Martin Konrad V4 (PD).gg
	{ .crc = 0x68979CF6, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Portrait - Pai Chan (J) [!].gg
	{ .crc = 0x695CC120, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Portrait - Yuuki Akira (J) [!].gg
	{ .crc = 0x51159F8F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Shinobi (E) [!].gg
	{ .crc = 0x30F1C984, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Shinobi (E) [b1].gg
	{ .crc = 0xE8AE1C44, .rom = 0x40005, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Shinobi (J).gg
	{ .crc = 0x83926BD1, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Turrican Demo V0.9 by Martin Konrad (PD).gg
	{ .crc = 0x39212762, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GG Turrican Demo V1.0 by Martin Konrad (PD).gg
	{ .crc = 0x971CA630, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GP Rider (J).gg
	{ .crc = 0x7A26EC6A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// GP Rider (U) [!].gg
	{ .crc = 0x876E9B72, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Galaga '91 (J) [b1].gg
	{ .crc = 0x124CD54F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Galaga '91 (J).gg
	{ .crc = 0x0593BA24, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gamble Panic (J).gg
	{ .crc = 0x09534742, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gambler Jikochuushin Ha (J).gg
	{ .crc = 0x423803A7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ganbare Gorby! (J).gg
	{ .crc = 0xA1F2F4A1, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Garfield - Caught in the Act (U) [!].gg
	{ .crc = 0xCD53F3AF, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gear Stadium Heiseiban (J) [!].gg
	{ .crc = 0xA0530664, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gear Stadium (J).gg
	{ .crc = 0x0E300223, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gear Works (U) [!].gg
	{ .crc = 0xE9A2EFB4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// George Foreman's KO Boxing (U) [!].gg
	{ .crc = 0x58B44585, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Godzilla - Kaiju Dai Shingeki (J) [!].gg
	{ .crc = 0x4CF97801, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Greendog (U) [b1].gg
	{ .crc = 0xC6B01820, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Greendog (U) [b2].gg
	{ .crc = 0x933E8D3A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Greendog (U).gg
	{ .crc = 0xF27925B0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Griffin (J).gg
	{ .crc = 0xA93E8B0F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Gunstar Heroes (J).gg
	{ .crc = 0xC3C52767, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Halley Wars (J).gg
	{ .crc = 0xDEF5A5D0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Halley Wars (U) [!].gg
	{ .crc = 0x7E9DEA46, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Head Buster (J) [b1].gg
	{ .crc = 0x069FB868, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Head Buster (J) [T+Eng.99_Chris Covell].gg
	{ .crc = 0x8ED94C2B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Head Buster (J).gg
	{ .crc = 0x7E689995, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Home Alone (U) [!].gg
	{ .crc = 0xDDE29F74, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Honoo no Doukyuuji - Dodge Danpei (J).gg
	{ .crc = 0xDFA805A0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Hook (U) [!].gg
	{ .crc = 0xF53CED2E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// House of Tarot (J) [!].gg
	{ .crc = 0x57834C03, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Hurricanes (U) [!].gg
	{ .crc = 0x0A25EEC5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Hyokkori Hyoutan Jima (J).gg
	{ .crc = 0x42389270, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Hyper Chou Pro Yakyuu '92 (J) [!].gg
	{ .crc = 0x056CAE74, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ichidant~R GG - Puzzle & Action (J).gg
	{ .crc = 0x9F64C2BB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// In the Wake of Vampire (J).gg
	{ .crc = 0xDAB0F265, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Incredible Crash Dummies, The (U).gg
	{ .crc = 0x087FC247, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Incredible Hulk, The (U) [!].gg
	{ .crc = 0xD7055F88, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Indiana Jones and the Last Crusade (U) [t1].gg
	{ .crc = 0x51AD2E27, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Indiana Jones and the Last Crusade (U).gg
	{ .crc = 0x8C048325, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Iron Man X-O Manowar in Heavy Metal (U) [a1].gg
	{ .crc = 0x847AE5CE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Iron Man X-O Manowar in Heavy Metal (U) [b1].gg
	{ .crc = 0xC1736457, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Iron Man X-O Manowar in Heavy Metal (U).gg
	{ .crc = 0x8927B69B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Itchy & Scratchy Game, The (U) [!].gg
	{ .crc = 0x44E7E2DA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// J.League Soccer - Dream Eleven (J) [!].gg
	{ .crc = 0xABDDF0EB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// James Bond 007 - The Duel (U) [!].gg
	{ .crc = 0x881A4524, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// James Pond 3 - Operation Starfish (U) [!].gg
	{ .crc = 0x68BB7F71, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// James Pond II - Codename RoboCod (U) [!].gg
	{ .crc = 0x9FB5C155, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jang Pung II (K) [S].gg
	{ .crc = 0x76C5BDFB, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Jeopardy! - Sports Edition (U) [!].gg
	{ .crc = 0x2DD850B7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jeopardy! (U) [!].gg
	{ .crc = 0xD7820C21, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Joe Montana's Football (JU) [!].gg
	{ .crc = 0x4A98678B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Judge Dredd (U) [!].gg
	{ .crc = 0x04D23FC4, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Judge Dredd (U) [b1].gg
	{ .crc = 0x9D1A05CD, .rom = 0x16A1B, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Junction (U).gg
	{ .crc = 0xA8EF36A7, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jungle Book, The (U) (Aug 1993) [!].gg
	{ .crc = 0x90100884, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jungle Book, The (U) (Mar 1994) [!].gg
	{ .crc = 0x30C09F31, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jurassic Park (J).gg
	{ .crc = 0x2F536AE3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Jurassic Park (UE) [!].gg
	{ .crc = 0xBD6F2321, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kaitou Saint Tail (J).gg
	{ .crc = 0x937FD52B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kawasaki Superbike Challenge (U) [!].gg
	{ .crc = 0x23F9150A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kenyuu Densetsu Yaiba (J).gg
	{ .crc = 0xD9CE3F4C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kick & Rush (J).gg
	{ .crc = 0xFD14CE00, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kinetic Connection (J) [o1].gg
	{ .crc = 0x808A71C3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kinetic Connection (J).gg
	{ .crc = 0x4AF7F2AA, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kishin Douji Zenki (J).gg
	{ .crc = 0x7D622BDD, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Klax (U) [!].gg
	{ .crc = 0x9B40FC8E, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kuni-chan no Game Tengoku Part 2 (J).gg
	{ .crc = 0xF3774C65, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Kuni-chan no Game Tengoku (J).gg
	{ .crc = 0x398F2358, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Land of Illusion Starring Mickey Mouse (J).gg
	{ .crc = 0x0117C3DF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Land of Illusion Starring Mickey Mouse (U) [!].gg
	{ .crc = 0x52DBF3E1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Land of Illusion Starring Mickey Mouse (U) [t1].gg
	{ .crc = 0xED79F845, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Last Action Hero (U) [!].gg
	{ .crc = 0x2D367C43, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Legend of Illusion Starring Mickey Mouse (J) [t1].gg
	{ .crc = 0x1194D5FB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Legend of Illusion Starring Mickey Mouse (J).gg
	{ .crc = 0xFE12A92F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Legend of Illusion Starring Mickey Mouse (U) [!].gg
	{ .crc = 0xCE5AD8B7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lemmings (Prototype) [!].gg
	{ .crc = 0x51548F61, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lemmings (U) [!].gg
	{ .crc = 0x0FDE7BAA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lemmings (U) [b1].gg
	{ .crc = 0xB316221E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lion King, The (E) [!].gg
	{ .crc = 0x0CD9C20B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lion King, The (E) [t1].gg
	{ .crc = 0x040512CE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lion King, The (U) [!].gg
	{ .crc = 0x9808D7B3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lion King, The (U) [b1].gg
	{ .crc = 0x64065877, .rom = 0x1F061, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lion King, The (U) [t1].gg
	{ .crc = 0x90D40776, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lost World, The - Jurassic Park (U).gg
	{ .crc = 0x8D1597F5, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lucky Dime Caper, The Starring Donald Duck (J).gg
	{ .crc = 0x2D98BD87, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lucky Dime Caper, The Starring Donald Duck (U) [!].gg
	{ .crc = 0x07A7815A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T+Bra].gg
	{ .crc = 0x5A558DA5, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T+Bra][a1].gg
	{ .crc = 0xEADA9B81, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T+Bra][b1].gg
	{ .crc = 0xE3C70AFD, .rom = 0x9888A, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng0.17_Naflign].gg
	{ .crc = 0x44E9CCAD, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng0.17_Naflign][b1].gg
	{ .crc = 0x91D82782, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-EngBeta1_Naflign].gg
	{ .crc = 0x4F7FE5FA, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-EngBeta2_Naflign].gg
	{ .crc = 0x2D6F9B05, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-EngBeta3_Naflign].gg
	{ .crc = 0xC77A28A1, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng].gg
	{ .crc = 0xEAF5732A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng][a1].gg
	{ .crc = 0x4A934DB5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng][a2].gg
	{ .crc = 0x208F0F87, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng][a2][b1].gg
	{ .crc = 0x8C390953, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J) [T-Eng][a3].gg
	{ .crc = 0x2AE16CBC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Lunar - Sanposuru Gakuen (J).gg
	{ .crc = 0x58459EDD, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MLBPA Baseball (U) [!].gg
	{ .crc = 0x1ECF07B4, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.0 - Secret of Monkey Island (PD).gg
	{ .crc = 0xF84D8CA8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.1 - Secret of Monkey Island (PD).gg
	{ .crc = 0x876555DE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.2 - Secret of Monkey Island (PD).gg
	{ .crc = 0xD5865194, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.3 - Secret of Monkey Island (PD).gg
	{ .crc = 0x465C8962, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.5 - Secret of Monkey Island (PD).gg
	{ .crc = 0x57A86D22, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.6 - Secret of Monkey Island (PD).gg
	{ .crc = 0x01903BBE, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.7 - Secret of Monkey Island (PD).gg
	{ .crc = 0x6CA4BA5E, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.8 - Secret of Monkey Island (PD).gg
	{ .crc = 0x6D31CC7B, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.3 - Shadow of the Beast (PD).gg
	{ .crc = 0xF998F57A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.5 - Shadow of the Beast (PD).gg
	{ .crc = 0x4F65A708, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.6 - Shadow of the Beast (PD).gg
	{ .crc = 0x535F1B6C, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.7 - Shadow of the Beast (PD).gg
	{ .crc = 0x2BFF1BBB, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// MOD2PSG V1.8 - Shadow of the Beast (PD).gg
	{ .crc = 0x08CDC425, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madden NFL '95 (U) [!].gg
	{ .crc = 0x75C71EBF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madden NFL '96 (U) [!].gg
	{ .crc = 0x75E273EB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madden NFL '96 (U) [b1].gg
	{ .crc = 0x9F6BA781, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari A - Doki Doki - Bake Shon (J).gg
	{ .crc = 0x7EC95282, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari I (J) [b1].gg
	{ .crc = 0x16ED3117, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari I (J).gg
	{ .crc = 0x00C34D94, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari II - Aruru 16-sai (J).gg
	{ .crc = 0x12EB2287, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari III - Kyukyoku Joo-sama (J) [a1].gg
	{ .crc = 0x568F4825, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Madou Monogatari III - Kyukyoku Joo-sama (J).gg
	{ .crc = 0x0A634D79, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Magic Knight Rayearth 2 - Making of Magic Knight (J) [!].gg
	{ .crc = 0x1C2C2B04, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Magic Knight Rayearth (J) [!].gg
	{ .crc = 0x8F82A6B9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Magical Puzzle Popils (J).gg
	{ .crc = 0xCF6D7BC5, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Magical Taruruuto-kun (J).gg
	{ .crc = 0x6E1CC23C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Majesco Game Gear BIOS (U) [!].gg
	{ .crc = 0x0EBEA9D4, .rom = 0x400, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Majors Pro Baseball, The (U) [!].gg
	{ .crc = 0x36EBCD6D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mappy (J).gg
	{ .crc = 0x01D2DD2A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Marble Madness (U) [b1].gg
	{ .crc = 0x94085B81, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Marble Madness (U) [o1].gg
	{ .crc = 0xFEB16091, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Marble Madness (U).gg
	{ .crc = 0x9559E339, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Marko's Magic Football (U) [!].gg
	{ .crc = 0x22C418BF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megaman (U) [!].gg
	{ .crc = 0x1ACE93AF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megaman (U) [b1].gg
	{ .crc = 0x6C8F46D6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megaman (U) [T+Ger1.00_Star-trans].gg
	{ .crc = 0x61ABD388, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megaman (U) [T-Ger_Star-trans].gg
	{ .crc = 0x1BEFCB82, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megami Tensei Gaiden - Last Bible Special (J).gg
	{ .crc = 0x4EC30806, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megami Tensei Gaiden - Last Bible (J) [b1].gg
	{ .crc = 0x2E8263D8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Megami Tensei Gaiden - Last Bible (J).gg
	{ .crc = 0x2E4EC17B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mick & Mack as the Global Gladiators (U) [!].gg
	{ .crc = 0xD2B6021E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mickey's Ultimate Challenge (U) [b1].gg
	{ .crc = 0x8BAB5634, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mickey's Ultimate Challenge (U).gg
	{ .crc = 0xECCF7A4F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Micro Machines 2 - Turbo Tournament (E).gg
	{ .crc = 0xDBE8895C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Micro Machines (E).gg
	{ .crc = 0xF7C524F6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Mighty Morphin Power Rangers - The Movie (U) [!].gg
	{ .crc = 0xB47C19E5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mighty Morphin Power Rangers (U) [!].gg
	{ .crc = 0x9289DFCC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Moldorian (J) [!].gg
	{ .crc = 0x4D5D15FB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Monster Truck Wars (U) [!].gg
	{ .crc = 0x453C5CEC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Monster World II - Dragon no Wana (J).gg
	{ .crc = 0xEA89E0E7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat 3 (U) [!].gg
	{ .crc = 0xC2BE62BB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat II (U) [!].gg
	{ .crc = 0x4B304E0F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat II (U) [b1].gg
	{ .crc = 0xFB85FC2A, .rom = 0x7F723, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat (J).gg
	{ .crc = 0xDBFF0461, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat (U) [!].gg
	{ .crc = 0x07494F2A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat (U) [b1].gg
	{ .crc = 0xFDEF9B5A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Mortal Kombat (U) [b2].gg
	{ .crc = 0xEBCBE485, .rom = 0x2B0B6, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ms. Pac-Man (U) [!].gg
	{ .crc = 0x5EE88BD5, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Action (U) [!].gg
	{ .crc = 0x19030108, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Jam Tournament Edition (U) [!].gg
	{ .crc = 0x86C32E5B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Jam Tournament Edition (U) [b1].gg
	{ .crc = 0x3600DCC8, .rom = 0x111F6, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Jam (J).gg
	{ .crc = 0xA49E9033, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Jam (U) (V1.0).gg
	{ .crc = 0x8F17597E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NBA Jam (U) (V1.1) [!].gg
	{ .crc = 0x820FA4AB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NFL '95 (U) [!].gg
	{ .crc = 0xDC5B0407, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NFL Quarterback Club '96 (U) [!].gg
	{ .crc = 0xC348E53A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NFL Quarterback Club (U) [!].gg
	{ .crc = 0x61785ED5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NHL All-Star Hockey (U) [!].gg
	{ .crc = 0x4680C7AA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NHL All-Star Hockey (U) [b1].gg
	{ .crc = 0xE4B5BB08, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// NHL Hockey (U) [!].gg
	{ .crc = 0x658713A5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Nazo Puyo 2 (J) [!].gg
	{ .crc = 0x73939DE4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Nazo Puyo Aruru no Ru (J) (Editor Prototype).gg
	{ .crc = 0x4C874466, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Nazo Puyo Aruru no Ru (J).gg
	{ .crc = 0x54AB42A4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Nazo Puyo (J) [!].gg
	{ .crc = 0xBCCE5FD4, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Neko Daisuki! (J).gg
	{ .crc = 0x3679BE80, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninja Gaiden (J) [!].gg
	{ .crc = 0x20EF017A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninja Gaiden (J) [b1].gg
	{ .crc = 0x4ED37A63, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninja Gaiden (U) [!].gg
	{ .crc = 0xC578756B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninja Gaiden (U) [T+Bra_Manao].gg
	{ .crc = 0x36925F37, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninja Gaiden (U) [T+Fre].gg
	{ .crc = 0x9266F987, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninku 2 - Tenkuryu-e no Michi (J) [!].gg
	{ .crc = 0x06247DD2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninku Gaiden - Hiroyuki Daikatsugeki (J).gg
	{ .crc = 0x9140F239, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninku (J) [!].gg
	{ .crc = 0xC3056E15, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ninku (J) [T+Eng0.1].gg
	{ .crc = 0x84C4B2E1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Nomo's World Series Baseball (J).gg
	{ .crc = 0x4ED45BDA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Olympic Gold - Barcelona '92 (E) (M8) [S][!].gg
	{ .crc = 0x1D93246E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Olympic Gold - Barcelona '92 (E) (M8) [S][b1].gg
	{ .crc = 0xCE97EFE8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Olympic Gold - Barcelona '92 (U) (M8) [S][!].gg
	{ .crc = 0xA2F9C7AF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ottifants, The (E).gg
	{ .crc = 0x1E673168, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun Europa (U) [S][!].gg
	{ .crc = 0xF037EC00, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun Europa (U) [S][b1].gg
	{ .crc = 0x2AA12D7E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun Europa (U) [S][T+Bra100%_Lohan].gg
	{ .crc = 0x86E5B455, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun Europa (U) [S][T+Bra].gg
	{ .crc = 0x0189931E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun (JU) [a1].gg
	{ .crc = 0x4753C309, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun (JU) [T+Bra100%_Lohan][b1].gg
	{ .crc = 0x00124D63, .rom = 0x20033, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun (JU) [T+Bra_TMT].gg
	{ .crc = 0x1738B458, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// OutRun (JU).gg
	{ .crc = 0xD58CB27C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour 96 (U) [!].gg
	{ .crc = 0x542B6D8E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf II (U) [!].gg
	{ .crc = 0x4A8AC851, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf II (U) [b1].gg
	{ .crc = 0xB4921239, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf II (U) [b2].gg
	{ .crc = 0x4E279BAA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf II (U) [b3].gg
	{ .crc = 0xFFF8CC8A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf (U) (V1.0) [!].gg
	{ .crc = 0x9700BB65, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// PGA Tour Golf (U) (V1.1) [!].gg
	{ .crc = 0x1C77D996, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Attack (U) [!].gg
	{ .crc = 0x9273EE2C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-In-Time (Prototype) [!].gg
	{ .crc = 0x64C28E20, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Man (USA, Europe).gg
	{ .crc = 0xA16C5E58, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Man (J) (GG2SMS V0.91 Hack).gg
	{ .crc = 0xFB7C7BF8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Man (J) (Partial GG2SMS by Chris Covell).gg
	{ .crc = 0xC945F35B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Man (J) [b1].gg
	{ .crc = 0x4C61517B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pac-Man (J).gg
	{ .crc = 0xA16C5E58, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Panzer Dragoon Mini (J) [!].gg
	{ .crc = 0xE9783CEA, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Paperboy 2 (U) [!].gg
	{ .crc = 0x8B2C454B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Paperboy (U) [t1].gg
	{ .crc = 0xA1365AE0, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Paperboy (U).gg
	{ .crc = 0xF54B6803, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pengo (J).gg
	{ .crc = 0xCE863DBA, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pengo (U) [!].gg
	{ .crc = 0x0DA23CC1, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pet Club Inu Daisuki! (J).gg
	{ .crc = 0xB42D8430, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pete Sampras Tennis (E) [!].gg
	{ .crc = 0xC1756BEE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J) [b1].gg
	{ .crc = 0xF8F4B9A3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J) [T+Eng1.01_AGTP].gg
	{ .crc = 0xCF1A8C1B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J) [T-Eng1.00_AGTP].gg
	{ .crc = 0x42C98A68, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J) [T-Eng_AGTP].gg
	{ .crc = 0xCEE31405, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J) [T-Eng_AGTP][a1].gg
	{ .crc = 0x88A3A56D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Adventure (J).gg
	{ .crc = 0x1A51579D, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [!].gg
	{ .crc = 0xA942514A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T+Bra_CBT].gg
	{ .crc = 0xD5AB83AF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T+Bra_CBT][a1].gg
	{ .crc = 0x8D555920, .rom = 0x40200, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T+EngV3.0Largefont_Magic_Trans].gg
	{ .crc = 0x988DAA36, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T+EngV3.0Smallfont_Magic_Trans].gg
	{ .crc = 0x93F3F062, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T+EngV4.0Smallfont_Magic_Trans].gg
	{ .crc = 0x1667748B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T-EngV1.0Largefont_Magic_Trans].gg
	{ .crc = 0x1C98154A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T-EngV1.0Smallfont_Magic_Trans].gg
	{ .crc = 0x17E64F1E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T-EngV2.0Largefont_Magic_Trans].gg
	{ .crc = 0xE9FB916B, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantasy Star Gaiden (J) [T-EngV2.0Smallfont_Magic_Trans].gg
	{ .crc = 0xE285CB3F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Phantom 2040 (U) [!].gg
	{ .crc = 0x281B1C3A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pinball Dreams (U) [!].gg
	{ .crc = 0x635C483A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pocket Jansou (J).gg
	{ .crc = 0xCC90C723, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pokemon Legends (Eternal Legend Hack).gg
	{ .crc = 0x7C5CD4A4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Poker Faced Paul's Blackjack (U) [!].gg
	{ .crc = 0x89D34067, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Poker Faced Paul's Gin (U) [!].gg
	{ .crc = 0xAFD61B89, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Poker Faced Paul's Poker (U) [!].gg
	{ .crc = 0xE783DAF8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Poker Faced Paul's Poker (U) [o1].gg
	{ .crc = 0x03BB0668, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Poker Faced Paul's Solitaire (U) [!].gg
	{ .crc = 0x0E9B0C0A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pop Breaker (J) (GG2SMS V1.0 Hack).gg
	{ .crc = 0xB37C3EA8, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pop Breaker (J) (GG2SMS V1.1 Hack).gg
	{ .crc = 0xBDFB09F6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pop Breaker (J).gg
	{ .crc = 0x71DEBA5A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Popeye's Beach Volleyball (J).gg
	{ .crc = 0x3EF66810, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Power Strike II (U) (GG2SMS V1.0 Hack).gg
	{ .crc = 0x979263D8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Power Strike II (U) [b1].gg
	{ .crc = 0x92945480, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Power Strike II (U).gg
	{ .crc = 0x09DE1528, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Predator 2 (U) [S][!].gg
	{ .crc = 0xE5F789B9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Primal Rage (U) [!].gg
	{ .crc = 0x2A34B5C7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Prince of Persia (E) [S][!].gg
	{ .crc = 0x45F058D6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Prince of Persia (U) [S][!].gg
	{ .crc = 0x311D2863, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Prince of Persia (U) [S][b1].gg
	{ .crc = 0xBA6344FC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Prince of Persia (U) [S][b2].gg
	{ .crc = 0x1C6C149C, .rom = 0x40046, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pro Yakyuu '91, The (J).gg
	{ .crc = 0x6D3A10D3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Pro Yakyuu GG League '94 (J).gg
	{ .crc = 0xA1A19135, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Psychic World (J) [o1].gg
	{ .crc = 0xE1109230, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Psychic World (J).gg
	{ .crc = 0xAFCC7828, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Psychic World (U) [!].gg
	{ .crc = 0x73779B22, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Putt & Putter (J).gg
	{ .crc = 0x407AC070, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Putt & Putter (U) [!].gg
	{ .crc = 0xECC301DD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Puyo Puyo 2 (J) [!].gg
	{ .crc = 0x3AB2393B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Puzlow Kids (J).gg
	{ .crc = 0xD173A06F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Puzzle Bobble (J).gg
	{ .crc = 0x8E54EE04, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Quiz Gear Fight!!, The (J).gg
	{ .crc = 0x736CDB76, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// R.B.I. Baseball '94 (U) [!].gg
	{ .crc = 0x6DC3295E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// R.B.I. Baseball '94 (U) [b1].gg
	{ .crc = 0xEF39F257, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// R.C. Grand Prix (U) [S][!].gg
	{ .crc = 0x56201996, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// R.C. Grand Prix (U) [S][b1].gg
	{ .crc = 0x4902B7A2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Rastan Saga (J) [S].gg
	{ .crc = 0x9C76FB3A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ren & Stimpy - Quest for the Shaven Yak, The (U) [!].gg
	{ .crc = 0x6C451EE1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Revenge of Drancon (U) [!].gg
	{ .crc = 0x03E9C607, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Riddick Bowe Boxing (J).gg
	{ .crc = 0xA45FFFB7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Riddick Bowe Boxing (U) [!].gg
	{ .crc = 0x38D8EC56, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Rise of the Robots (U) [!].gg
	{ .crc = 0x100B77B2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ristar the Shooting Star (U) [!].gg
	{ .crc = 0xEFE65B3B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Road Rash (U).gg
	{ .crc = 0x96045F76, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Robocop 3 (U) [!].gg
	{ .crc = 0x069A0704, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Robocop versus The Terminator (U) [!].gg
	{ .crc = 0x4AB7FA4E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Robocop versus The Terminator (U) [b1].gg
	{ .crc = 0xA74C4CC0, .rom = 0x2E4EE, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ronald in the Magical World (JU) [t1].gg
	{ .crc = 0x0F84BD44, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ronald in the Magical World (JU).gg
	{ .crc = 0x87B8B612, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Royal Stone (J).gg
	{ .crc = 0x445D7CD2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ryu Kyu (J).gg
	{ .crc = 0x95EFD52B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// S.S. Lucifer - Man Overboard! (U) [!].gg
	{ .crc = 0xD9A7F170, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_CODEMASTERS, .sys = SMS_System_GG },
	// SD Gundam - Winner's History (J) [T+Eng].gg
	{ .crc = 0xD9A69E9E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// SD Gundam - Winner's History (J) [T+Fre0.99_Asmodeath].gg
	{ .crc = 0x877326BC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// SD Gundam - Winner's History (J).gg
	{ .crc = 0x5E2B39B8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// SMSC Gamegear Demo (PD) [b1].gg
	{ .crc = 0x5AAC063F, .rom = 0x10001, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Samurai Shodown (U) [!].gg
	{ .crc = 0x98171DEB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Samurai Spirits (J) [b1].gg
	{ .crc = 0x236990E0, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Samurai Spirits (J).gg
	{ .crc = 0x93FD73DC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Scratch Golf (JU) [!].gg
	{ .crc = 0xEC0F2C72, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sega Game Pack 4 in 1 (U) [!].gg
	{ .crc = 0x0924D2EC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sega Game Pack 4 in 1 (U) [b1].gg
	{ .crc = 0xD06136AC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sega Tween (Normal) by Ben Ryves (PD).gg
	{ .crc = 0x2ABD4E44, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sensible Soccer (E).gg
	{ .crc = 0x5B68DA75, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shadam Crusader (J) [!].gg
	{ .crc = 0x09F9ED60, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shanghai II (J).gg
	{ .crc = 0x2AE8C75F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shaq Fu (U) [!].gg
	{ .crc = 0x6FCB8AB0, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shikinjou (J).gg
	{ .crc = 0x9C5C7F53, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [b1].gg
	{ .crc = 0x7BF365E1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [b1][T-Eng].gg
	{ .crc = 0x6F70B6CF, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [b2].gg
	{ .crc = 0x2A89AA37, .rom = 0x5740E, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [b3].gg
	{ .crc = 0x5356F6D7, .rom = 0x5740E, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [T+Eng29Nov2005_SFC].gg
	{ .crc = 0x23465DFB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [T-Eng23Aug2005_SFC].gg
	{ .crc = 0x51AE00D4, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [T-Eng].gg
	{ .crc = 0xD8138501, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [T-Eng][a1].gg
	{ .crc = 0x749A2D70, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J) [T-Eng][a2].gg
	{ .crc = 0xF8DD97DC, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden - Final Conflict (J).gg
	{ .crc = 0x6019FE5E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden II (J).gg
	{ .crc = 0x30374681, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force Gaiden (J) [!].gg
	{ .crc = 0x4D1F4699, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force II - The Sword of Hajya (U) [!].gg
	{ .crc = 0xA6CA6FA9, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shining Force II - The Sword of Hajya (U) [b1].gg
	{ .crc = 0x56A51FF2, .rom = 0x63692, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shinobi II - The Silent Fury (U) [!].gg
	{ .crc = 0x6201C694, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shinobi II - The Silent Fury (U) [b1].gg
	{ .crc = 0xB2ADC6B4, .rom = 0x33115, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shinobi II - The Silent Fury (U) [b2].gg
	{ .crc = 0x1E03DB14, .rom = 0x39BE8, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shinobi II - The Silent Fury (U) [b3].gg
	{ .crc = 0xC2D8FE07, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Shinobi II - The Silent Fury (U) [b4].gg
	{ .crc = 0x988E9234, .rom = 0x3F89B, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Side Pocket (U).gg
	{ .crc = 0x6A603EED, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simple Text Demo by Charles Doty (PD) [a1].gg
	{ .crc = 0x05C3F2DF, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simple Text Demo by Charles Doty (PD) [o1].gg
	{ .crc = 0x27DF7B01, .rom = 0x10001, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simple Text Demo by Charles Doty (PD).gg
	{ .crc = 0xD250DB57, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Bart vs. The Space Mutants (U) [!].gg
	{ .crc = 0xC0009274, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Bart vs. The Space Mutants (U) [t1].gg
	{ .crc = 0x9DE396CA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Bart vs. The World (U) [!].gg
	{ .crc = 0xDA7BD5C7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Bart vs. The World (U) [b1].gg
	{ .crc = 0x18BA70D3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Bartman Meets Radioactive Man (U) [!].gg
	{ .crc = 0xFFA447A9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Simpsons, The - Krusty's Fun House (U) [!].gg
	{ .crc = 0xD01E784F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Skweek (J) [!].gg
	{ .crc = 0x786DD67B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Slider (U).gg
	{ .crc = 0x4DC6F555, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Smurfs, The (E) [!].gg
	{ .crc = 0x354E1CBD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Solitaire Funpak (U) [!].gg
	{ .crc = 0xF6F24B75, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Solitaire Poker (U) [!].gg
	{ .crc = 0x06F2FC46, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic & Tails 2 (J) [t1].gg
	{ .crc = 0x2846BBB3, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic & Tails 2 (J).gg
	{ .crc = 0x496BCE64, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic & Tails (J) [!].gg
	{ .crc = 0x8AC0DADE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Blast (U) [!].gg
	{ .crc = 0x031B9DA9, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Chaos (U) [!].gg
	{ .crc = 0x663F2ABB, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Chaos (U) [b1].gg
	{ .crc = 0x965705F1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Drift 2 (J) [!].gg
	{ .crc = 0xD6E8A305, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Drift 2 (J) [b1].gg
	{ .crc = 0x7CA02906, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Drift 2 (J) [o1].gg
	{ .crc = 0x7C8ABCC4, .rom = 0x80040, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Drift (J).gg
	{ .crc = 0x68F0A776, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Labyrinth (U) [!].gg
	{ .crc = 0x5550173B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Labyrinth (U) [b1].gg
	{ .crc = 0x3AD79969, .rom = 0x6980A, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Labyrinth (U) [b2].gg
	{ .crc = 0x27160662, .rom = 0x7ECD6, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic Spinball (U) [!].gg
	{ .crc = 0xA9210434, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog - Triple Trouble (U) [!].gg
	{ .crc = 0xD23A2A93, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog 2 (U) [!].gg
	{ .crc = 0x95A18EC7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog 2 (U) [b1].gg
	{ .crc = 0x825AE8B4, .rom = 0x80002, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog (Prototype) [!].gg
	{ .crc = 0x816C0A1E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog (U) (V1.0) [!].gg
	{ .crc = 0x3E31CB8C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sonic The Hedgehog (U) (V1.1) [!].gg
	{ .crc = 0xD163356E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Soukoban (J).gg
	{ .crc = 0x0F3E3840, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Space Harrier (JU) [!].gg
	{ .crc = 0x600C15B3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Spider-Man - Return of the Sinister Six (U).gg
	{ .crc = 0xBC240779, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Spider-Man and the X-Men in Arcade's Revenge (U) [!].gg
	{ .crc = 0x742A372B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Spider-Man vs. The Kingpin (U) [!].gg
	{ .crc = 0x2651024E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sports Illustrated Championship Football & Baseball (U) [!].gg
	{ .crc = 0xDE25E2D8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sports Trivia - Championship Edition (U).gg
	{ .crc = 0x5B5DE94D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sports Trivia (U) [!].gg
	{ .crc = 0xA7AF7CA9, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Star Trek Generations - Beyond the Nexus (U) [!].gg
	{ .crc = 0x087FC5F0, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Star Trek TNG - Advanced Holodeck Tutorial (U) [!].gg
	{ .crc = 0x80156323, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Star Wars (U) [!].gg
	{ .crc = 0x0228769C, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Star Wars (U) [b1].gg
	{ .crc = 0xBD309D97, .rom = 0x7ECB8, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Stargate (U).gg
	{ .crc = 0xFC7C64E4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Streets of Rage II (U) [!].gg
	{ .crc = 0x0B618409, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Streets of Rage (U) [!].gg
	{ .crc = 0x3D8BCF1D, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Strider Returns (U) [!].gg
	{ .crc = 0x1EBFA5CA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Battletank (U) [!].gg
	{ .crc = 0x73D6745A, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Columns (J) [!].gg
	{ .crc = 0x2A100717, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Columns (U) [!].gg
	{ .crc = 0x8BA43AF3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Golf (J).gg
	{ .crc = 0x528CBBCE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Kick Off (J) [S][!].gg
	{ .crc = 0x10DBBEF4, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Monaco GP (J) [b1].gg
	{ .crc = 0xE30E7DC6, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Monaco GP (J).gg
	{ .crc = 0x4F686C4A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Monaco GP (U) [!].gg
	{ .crc = 0xFCF12547, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Off-Road (U) [a1].gg
	{ .crc = 0xE8B42B1F, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Off-Road (U) [b1].gg
	{ .crc = 0xA8432D2B, .rom = 0x1F758, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Off-Road (U).gg
	{ .crc = 0x2E217FAE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Return of the Jedi (U) [!].gg
	{ .crc = 0x4A38B6B6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Smash T.V. (U) [!].gg
	{ .crc = 0x1006E4E3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Space Invaders (U) [!].gg
	{ .crc = 0xDFE38E24, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Space Invaders (U) [b1].gg
	{ .crc = 0x2D8E81BC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Tetris (K) [S][!].gg
	{ .crc = 0xBD1CC7DF, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Super Tetris (K) [S][o1].gg
	{ .crc = 0x8230384E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Superman (U) [!].gg
	{ .crc = 0x73DF5A15, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Surf Ninjas (U) [!].gg
	{ .crc = 0x284482A8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Surf Ninjas (U) [b1].gg
	{ .crc = 0x18DEEEC8, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Surf Ninjas (U) [b2].gg
	{ .crc = 0x2EA26930, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [!].gg
	{ .crc = 0x45EF2062, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [h1] (Status Music).gg
	{ .crc = 0x8C133DC5, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [T+Eng1.00_AGTP].gg
	{ .crc = 0x91197F9F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [T+Eng1.00_AGTP][h1] (Status Music).gg
	{ .crc = 0x58E56238, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [T+Eng1.01_AGTP].gg
	{ .crc = 0x947C2921, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [T+Eng1.01_AGTP][h1] (Status Music).gg
	{ .crc = 0x5D803486, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Sylvan Tale (J) [T+Fre0.9_Asmodeath].gg
	{ .crc = 0xD6880A56, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// T2 - The Arcade Game (U) [!].gg
	{ .crc = 0x9479C83A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// T2 - The Arcade Game (U) [b1].gg
	{ .crc = 0x6DAAD1A1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tails' Adventures (U) [!].gg
	{ .crc = 0x5BB6E5D6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tails' Adventures (U) [b1].gg
	{ .crc = 0xCB591501, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tails' Sky Patrol (J) [!].gg
	{ .crc = 0x88618AFA, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tails' Sky Patrol (J) [T+Bra].gg
	{ .crc = 0xDB22F3CF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taisen Mahjong HaoPai 2 (J).gg
	{ .crc = 0x20527530, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taisen Mahjong HaoPai (J).gg
	{ .crc = 0xCF9C607C, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taisen-gata Daisenryaku G (J).gg
	{ .crc = 0x7B7717B8, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tale Spin (U) [!].gg
	{ .crc = 0xF1732FFE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tama & Friends - 3choume Kouen - Tamalympics (J) [b1].gg
	{ .crc = 0xCEA67B87, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tama & Friends - 3choume Kouen - Tamalympics (J).gg
	{ .crc = 0xDD1D2EBF, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tanto-R (J).gg
	{ .crc = 0x09151743, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taz in Escape from Mars (U) (Star Wars Text Hack).gg
	{ .crc = 0x7F278AB2, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taz in Escape from Mars (U) [!].gg
	{ .crc = 0xEEBAD66B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taz in Escape from Mars (U) [t1].gg
	{ .crc = 0x60D5ED5D, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taz-Mania - The Search for the Lost Seabirds (U) [!].gg
	{ .crc = 0x36040C24, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Taz-Mania - The Search for the Lost Seabirds (U) [b1].gg
	{ .crc = 0x183C0BCC, .rom = 0x71E8E, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tempo Jr. (U) [!].gg
	{ .crc = 0xDE466796, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Terminator 2 - Judgment Day (U).gg
	{ .crc = 0x1BD15773, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Terminator, The (U) [!].gg
	{ .crc = 0xC029A5FD, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tesserae (U) [!].gg
	{ .crc = 0xCA0E11CC, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tesserae (U) [b1].gg
	{ .crc = 0xBF696F94, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tintin au Tibet (E) [!].gg
	{ .crc = 0x969CF389, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tom and Jerry - The Movie (U) [!].gg
	{ .crc = 0x5CD33FF2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Tom and Jerry - The Movie (U) [t1].gg
	{ .crc = 0xB75416AE, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Torarete Tamaruka! (J).gg
	{ .crc = 0x5BCF9B97, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// True Lies (U).gg
	{ .crc = 0x5173B02A, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Twisty by Charles MacDonald (PD).gg
	{ .crc = 0x9AA9ED07, .rom = 0xC000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Ultimate Soccer (E).gg
	{ .crc = 0x820965A3, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Urban Strike (U) [!].gg
	{ .crc = 0x185E9FC1, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// VR Troopers (U) [!].gg
	{ .crc = 0xB0F22745, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// VR Troopers (U) [b1].gg
	{ .crc = 0x231E5B78, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Vampire - Master of Darkness (U) [!].gg
	{ .crc = 0x7EC64025, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Vampire - Master of Darkness (U) [t1].gg
	{ .crc = 0xE6EA018F, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Virtua Fighter Animation (U) [!].gg
	{ .crc = 0xD431C452, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Virtua Fighter Animation (U) [b1].gg
	{ .crc = 0x924CE5BE, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Virtua Fighter Mini (J) [!].gg
	{ .crc = 0xC05657F8, .rom = 0x100000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// WWF Raw (U) [!].gg
	{ .crc = 0x8DC68D92, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// WWF Raw (U) [b1].gg
	{ .crc = 0xD29148A9, .rom = 0x44000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// WWF Steel Cage Challenge (U) [S][!].gg
	{ .crc = 0xDA8E95A9, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wagyan Land (J) [a1].gg
	{ .crc = 0xD5369192, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wagyan Land (J) [b1].gg
	{ .crc = 0xF514358C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wagyan Land (J) [b2].gg
	{ .crc = 0x9EE91147, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wagyan Land (J) [b3].gg
	{ .crc = 0x8C33319C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wagyan Land (J).gg
	{ .crc = 0x29E697B2, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wheel of Fortune (U) [!].gg
	{ .crc = 0xE91CDB2A, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wimbledon (U) [!].gg
	{ .crc = 0xCE1108FD, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Windows for GG by Victor Kemp (PD).gg
	{ .crc = 0xF366B29D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Winter Olympics - Lillehammer '94 (J) [!].gg
	{ .crc = 0xD5195A39, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Winter Olympics - Lillehammer '94 (U) [!].gg
	{ .crc = 0xD15D335B, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wizard Pinball (U) [!].gg
	{ .crc = 0x9E03F96C, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wolfchild (U) [!].gg
	{ .crc = 0x840A8F8E, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wolfchild (U) [T+Spa100_Pkt][b1].gg
	{ .crc = 0x69DE95AC, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wonder Boy - The Dragon's Trap (E) [!].gg
	{ .crc = 0xA74C97A7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wonder Boy - The Dragon's Trap (Prototype).gg
	{ .crc = 0xDB1B5B44, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wonder Boy (E) [!].gg
	{ .crc = 0xEA2DD3A7, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Wonder Boy (J).gg
	{ .crc = 0x9977FCB3, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Woody Pop (U) (V1.0).gg
	{ .crc = 0x9C0E5A04, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Woody Pop (U) (V1.1) [!].gg
	{ .crc = 0xB74F3A4F, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Class Leader Golf (U) [!].gg
	{ .crc = 0x868FE528, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Cup 94 (U) [!].gg
	{ .crc = 0xD2BB3690, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Cup Soccer (U) [!].gg
	{ .crc = 0xDD6D2E34, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Derby (J).gg
	{ .crc = 0x1E81861F, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Series Baseball '95 (U) [!].gg
	{ .crc = 0x578A8A38, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Series Baseball (U) (V1.0) [!].gg
	{ .crc = 0x3D8D0DD6, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// World Series Baseball (U) (V1.1) [!].gg
	{ .crc = 0xBB38CFD7, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// X-Men - Gamemaster's Legacy (U) [!].gg
	{ .crc = 0xC169C344, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// X-Men - Mojo World (U) [!].gg
	{ .crc = 0xC2CBA9D7, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// X-Men (U) [!].gg
	{ .crc = 0x567A5EE6, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Xaropinho (Mappy Hack).gg
	{ .crc = 0xDD03E6DE, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Yogi Bear in Yogi Bear's Goldrush (Prototype) [!].gg
	{ .crc = 0xE678F264, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Yuu Yuu Hakusho II (J) [T+Eng].gg
	{ .crc = 0xDCBF3355, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Yuu Yuu Hakusho II (J).gg
	{ .crc = 0x46AE9159, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Yuu Yuu Hakusho (J).gg
	{ .crc = 0x88EBBF9E, .rom = 0x80000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zan Gear (J).gg
	{ .crc = 0x141AAF96, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zool (J).gg
	{ .crc = 0xE35EF7ED, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zool (U) [!].gg
	{ .crc = 0xB287C695, .rom = 0x40000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zoop'em Up by Martin Konrad (PD).gg
	{ .crc = 0x590F9C54, .rom = 0x10000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zoop (U) [!].gg
	{ .crc = 0x3247FF8B, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },
	// Zoop (U) [a1][!].gg
	{ .crc = 0xF397F041, .rom = 0x20000, .ram = 0x0000, .map = MAPPER_TYPE_SEGA, .sys = SMS_System_GG },

// SG-1000
/* --------------------------------------------------------------------------------------------------------- */
    // Bank Panic (Japan).sg
	{ .crc = 0xD8A87095, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Bomb Jack (Japan).sg
	{ .crc = 0xEA0F2691, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Bomberman Special (Taiwan).sg
	{ .crc = 0x69FC1494, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_B, .sys = SMS_System_SG1000 },
	// Borderline (Japan, Europe).sg
	{ .crc = 0x0B4BCA74, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// C_So! (Japan).sg
	{ .crc = 0xBE7ED0EB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Cabbage Patch Kids (TW).sg
	{ .crc = 0x9D91AB78, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Castle, The (Japan).sg
	{ .crc = 0x092F29D6, .rom = 0x8000, .ram = 0x2000, .map = MAPPER_TYPE_THE_CASTLE, .sys = SMS_System_SG1000 },
	// Castle, The (Taiwan).sg
    { .crc = 0x2E366CCF, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_B, .sys = SMS_System_SG1000 },
    // Chack'n Pop (Japan).sg
	{ .crc = 0xD37BDA49, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Baseball (Japan) (16kB).sg
	{ .crc = 0x5970A12B, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Billiards (Japan).sg
	{ .crc = 0x62B21E31, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Boxing (Japan).sg
	{ .crc = 0x26F947D1, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Golf (Japan).sg
	{ .crc = 0x868419B5, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Ice Hockey (Japan).sg
	{ .crc = 0xBDC05652, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Kendou (Japan).sg
	{ .crc = 0x10CDEBCE, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Pro Wrestling (Japan).sg
	{ .crc = 0x372FE6BC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Soccer (Japan).sg
	{ .crc = 0x6F39719E, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Champion Tennis (Japan).sg
	{ .crc = 0x7C663316, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Championship Lode Runner (Japan).sg
	{ .crc = 0x11DB4B1D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Choplifter (Japan).sg
	{ .crc = 0x732B7180, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Circus Charlie (KR).sg
	{ .crc = 0x7F7F009D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Congo Bongo (SG-1000).sg
	{ .crc = 0xDC4383CC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Doki Doki Penguin Land (SG-1000).sg
	{ .crc = 0xFDC095BC, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Dragon Wang (Japan).sg
	{ .crc = 0x99C3DE21, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Drol (Japan).sg
	{ .crc = 0x288940CB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Elevator Action (Japan).sg
	{ .crc = 0x5AF8F69D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Exerion (Japan, Europe).sg
	{ .crc = 0xA2C45B61, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Flicky (Japan).sg
	{ .crc = 0xBD24D27B, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Flipper (Taiwan).sg
	{ .crc = 0x042C36BA, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// GP World (Japan).sg
	{ .crc = 0x942ADF84, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Galaga (Taiwan).sg
	{ .crc = 0x845BBB22, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Girl's Garden (Japan).sg
	{ .crc = 0x1898F274, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Golgo 13 (Japan).sg
	{ .crc = 0x0D159ED0, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Gulkave (Japan).sg
	{ .crc = 0x15A754A3, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Guzzler (Japan) (Othello Multivision).sg
	{ .crc = 0x61FA9EA0, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// H.E.R.O. (Japan).sg
	{ .crc = 0x4587DE6E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Hang-On II (Japan).sg
	{ .crc = 0x9BE3C6BD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Hustle Chumy (Japan).sg
	{ .crc = 0xA627D440, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Hyper Sports (Japan).sg
	{ .crc = 0xBA09A0FD, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Hyper Sports 2 (TW).sg
	{ .crc = 0xB0234E12, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// James Bond 007 (SG-1000) [!].sg
	{ .crc = 0x90160849, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// King's Valley (Taiwan).sg
	{ .crc = 0x223397A1, .rom = 0x8000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Knightmare (Taiwan).sg
	{ .crc = 0x281D2888, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Legend of Kage, The (Taiwan).sg
	{ .crc = 0x2E7166D5, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Lode Runner (Japan, Europe).sg
	{ .crc = 0x00ED3970, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Magical Kid Wiz (Taiwan).sg
	{ .crc = 0xFFC4EE3F, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Magical Tree (TW).sg
	{ .crc = 0xB3A8291A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Monaco GP (SG-1000).sg
	{ .crc = 0x72542786, .rom = 0xA000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// N-Sub (Europe).sg
	{ .crc = 0x09196FC5, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Ninja Princess (Japan).sg
	{ .crc = 0x3B912408, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Orguss (Japan, Europe).sg
	{ .crc = 0xF4F78B76, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Othello (Japan).sg
	{ .crc = 0xAF4F14BC, .rom = 0x8000, .ram = 0x0800, .map = MAPPER_TYPE_OTHELLO, .sys = SMS_System_SG1000 },
	// Pacar (Japan, Europe).sg
	{ .crc = 0x30C52E5E, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Pachinko (Japan).sg
	{ .crc = 0x326587E1, .rom = 0xA000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Pachinko II (Japan).sg
	{ .crc = 0xFD7CB50A, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Pitfall II - The Lost Caverns (Japan).sg
	{ .crc = 0x37FCA2EB, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Pop Flamer (Japan, Europe).sg
	{ .crc = 0xDB6404BA, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Q-bert (Japan) (Othello Multivision).sg
	{ .crc = 0x77DB4704, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Rally-X (Taiwan) (DahJee).sg
	{ .crc = 0x306D5F78, .rom = 0x8000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Road Fighter (Taiwan) (Jumbo).sg
	{ .crc = 0x29E047CC, .rom = 0x8000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Rock n' Bolt (Japan).sg
	{ .crc = 0x0FFDD03D, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Safari Hunting (Japan).sg
	{ .crc = 0x49E9718B, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Safari Race (Europe).sg
	{ .crc = 0x619DD066, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Shinnyushain Tooru Kun (Japan).sg
	{ .crc = 0x5A917E06, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Sindbad Mystery (Japan, Europe).sg
	{ .crc = 0x01932DF9, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Soukoban (Japan).sg
	{ .crc = 0x922C5468, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Space Armor (Japan) (v2.0) (Newer) (Othello Multivision).sg
	{ .crc = 0xAC4F0A5C, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Space Invaders (Japan).sg
	{ .crc = 0x6AD5CB3D, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Space Mountain (Japan) (Othello Multivision).sg
	{ .crc = 0xBBD87D8F, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Space Slalom (Japan).sg
	{ .crc = 0xB8B58B30, .rom = 0x2000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Star Force (Japan).sg
	{ .crc = 0xB846B52A, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Star Jacker (Japan, Europe) (Rev 2).sg
	{ .crc = 0x3FE59505, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Super Tank (Japan).sg
	{ .crc = 0x084CC13E, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Tank Battalion (Taiwan).sg
	{ .crc = 0x5CBD1163, .rom = 0x8000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// TwinBee (Taiwan).sg
	{ .crc = 0xC550B4F0, .rom = 0xC000, .ram = 0x2000, .map = MAPPER_TYPE_DAHJEE_A, .sys = SMS_System_SG1000 },
	// Wonder Boy (Japan).sg
	{ .crc = 0x160535C5, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Yamato (Japan, Europe).sg
	{ .crc = 0xE2FD5201, .rom = 0x4000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Zaxxon (Japan).sg
	{ .crc = 0x905467E4, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Zippy Race (Japan).sg
	{ .crc = 0xBC5D20DF, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
	// Zoom 909 (Japan).sg
	{ .crc = 0x093830D8, .rom = 0x8000, .ram = 0x0000, .map = MAPPER_TYPE_NONE, .sys = SMS_System_SG1000 },
};

bool rom_database_find_entry(struct RomEntry* entry, const uint32_t crc)
{
    for (size_t i = 0; i < ARRAY_SIZE(ENTRIES); i++)
    {
        if (ENTRIES[i].crc == crc)
        {
            *entry = ENTRIES[i];
            return true;
        }
    }

    return false;
}
