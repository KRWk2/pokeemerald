#include "global.h"
#include "gba/m4a_internal.h"
#include "sound.h"
#include "battle.h"
#include "m4a.h"
#include "main.h"
#include "pokemon.h"
#include "constants/songs.h"
#include "task.h"

struct Fanfare
{
    u16 songNum;
    u16 duration;
};

EWRAM_DATA struct MusicPlayerInfo* gMPlay_PokemonCry = NULL;
EWRAM_DATA u8 gPokemonCryBGMDuckingCounter = 0;

static u16 sCurrentMapMusic;
static u16 sNextMapMusic;
static u8 sMapMusicState;
static u8 sMapMusicFadeInSpeed;
static u16 sFanfareCounter;

bool8 gDisableMusic;

extern struct ToneData gCryTable[];
extern struct ToneData gCryTable_Reverse[];

static void Task_Fanfare(u8 taskId);
static void CreateFanfareTask(void);
static void Task_DuckBGMForPokemonCry(u8 taskId);
static void RestoreBGMVolumeAfterPokemonCry(void);

static const struct Fanfare sFanfares[] = {
    [FANFARE_LEVEL_UP]                = { MUS_LEVEL_UP,               80 },
    [FANFARE_HEAL]                    = { MUS_HEAL,                  160 },
    [FANFARE_OBTAIN_BADGE]            = { MUS_OBTAIN_BADGE,          340 },
    [FANFARE_OBTAIN_ITEM]             = { MUS_OBTAIN_ITEM,           160 },
    [FANFARE_EVOLVED]                 = { MUS_EVOLVED,               220 },
    [FANFARE_OBTAIN_TMHM]             = { MUS_OBTAIN_TMHM,           220 },
    [FANFARE_EVOLUTION_INTRO]         = { MUS_EVOLUTION_INTRO,        60 },
    [FANFARE_MOVE_DELETED]            = { MUS_MOVE_DELETED,          180 },
    [FANFARE_OBTAIN_BERRY]            = { MUS_OBTAIN_BERRY,          120 },
    [FANFARE_AWAKEN_LEGEND]           = { MUS_AWAKEN_LEGEND,         710 },
    [FANFARE_SLOTS_JACKPOT]           = { MUS_SLOTS_JACKPOT,         250 },
    [FANFARE_SLOTS_WIN]               = { MUS_SLOTS_WIN,             150 },
    [FANFARE_TOO_BAD]                 = { MUS_TOO_BAD,               160 },
    [FANFARE_RG_JIGGLYPUFF]           = { MUS_RG_JIGGLYPUFF,         400 },
    [FANFARE_RG_DEX_RATING]           = { MUS_RG_DEX_RATING,         196 },
    [FANFARE_RG_OBTAIN_KEY_ITEM]      = { MUS_RG_OBTAIN_KEY_ITEM,    170 },
    [FANFARE_RG_CAUGHT_INTRO]         = { MUS_RG_CAUGHT_INTRO,       231 },
    [FANFARE_RG_PHOTO]                = { MUS_RG_PHOTO,               75 },
    [FANFARE_RG_POKE_FLUTE]           = { MUS_RG_POKE_FLUTE,         450 },
    [FANFARE_OBTAIN_B_POINTS]         = { MUS_OBTAIN_B_POINTS,       313 },
    [FANFARE_REGISTER_MATCH_CALL]     = { MUS_REGISTER_MATCH_CALL,   135 },
    [FANFARE_OBTAIN_SYMBOL]           = { MUS_OBTAIN_SYMBOL,         318 },
    [FANFARE_HG_OBTAIN_KEY_ITEM]      = { HG_SEQ_ME_KEYITEM,         170 },
    [FANFARE_HG_LEVEL_UP]             = { HG_SEQ_ME_LVUP,             80 },
    [FANFARE_HG_HEAL]                 = { HG_SEQ_ME_ASA,             160 },
    [FANFARE_HG_DEX_RATING_1]         = { HG_SEQ_ME_HYOUKA1,         200 },
    [FANFARE_HG_DEX_RATING_2]         = { HG_SEQ_ME_HYOUKA2,         180 },
    [FANFARE_HG_DEX_RATING_3]         = { HG_SEQ_ME_HYOUKA3,         220 },
    [FANFARE_HG_DEX_RATING_4]         = { HG_SEQ_ME_HYOUKA4,         210 },
    [FANFARE_HG_DEX_RATING_5]         = { HG_SEQ_ME_HYOUKA5,         210 },
    [FANFARE_HG_DEX_RATING_6]         = { HG_SEQ_ME_HYOUKA6,         370 },
    [FANFARE_HG_RECEIVE_EGG]          = { HG_SEQ_ME_TAMAGO_GET,      155 },
    [FANFARE_HG_OBTAIN_ITEM]          = { HG_SEQ_ME_ITEM,            160 },
    [FANFARE_HG_EVOLVED]              = { HG_SEQ_ME_SHINKAOME,       240 },
    [FANFARE_HG_OBTAIN_BADGE]         = { HG_SEQ_ME_BADGE,           340 },
    [FANFARE_HG_OBTAIN_TMHM]          = { HG_SEQ_ME_WAZA,            220 },
    [FANFARE_HG_VOLTORB_FLIP_1]       = { HG_SEQ_ME_CARDGAME1,       195 },
    [FANFARE_HG_VOLTORB_FLIP_2]       = { HG_SEQ_ME_CARDGAME2,       240 },
    [FANFARE_HG_ACCESSORY]            = { HG_SEQ_ME_ACCE,            160 },
    [FANFARE_HG_REGISTER_POKEGEAR]    = { HG_SEQ_ME_POKEGEAR_REGIST, 185 },
    [FANFARE_HG_OBTAIN_BERRY]         = { HG_SEQ_ME_KINOMI,          120 },
    [FANFARE_HG_RECEIVE_POKEMON]      = { HG_SEQ_ME_PT_SPECIAL,      150 },
    [FANFARE_HG_MOVE_DELETED]         = { HG_SEQ_ME_WASURE,          180 },
    [FANFARE_HG_THIRD_PLACE]          = { HG_SEQ_ME_MUSHITORI3,      130 },
    [FANFARE_HG_SECOND_PLACE]         = { HG_SEQ_ME_MUSHITORI2,      225 },
    [FANFARE_HG_FIRST_PLACE]          = { HG_SEQ_ME_MUSHITORI1,      250 },
    [FANFARE_HG_POKEATHLON_NEW]       = { HG_SEQ_ME_PT_NEW,          110 },
    [FANFARE_HG_WINNING_POKEATHLON]   = { HG_SEQ_ME_PT_RESULTG,      144 },
    [FANFARE_HG_OBTAIN_B_POINTS]      = { HG_SEQ_ME_BPGET,           264 },
    [FANFARE_HG_OBTAIN_ARCADE_POINTS] = { HG_SEQ_ME_ROULETTE,        175 },
    [FANFARE_HG_OBTAIN_CASTLE_POINTS] = { HG_SEQ_ME_CASTLE,          200 },
    [FANFARE_HG_CLEAR_MINIGAME]       = { HG_SEQ_ME_MINIGAME,        230 },
    [FANFARE_HG_PARTNER]              = { HG_SEQ_ME_GONIN,           180 },
    [FANFARE_BW_OBTAIN_KEY_ITEM]      = { BW_SEQ_ME_KEYITEM,         170 },
    [FANFARE_BW_LEVEL_UP]             = { BW_SEQ_ME_LVUP,             80 },
    [FANFARE_BW_HEAL]                 = { BW_SEQ_ME_ASA,             160 },
    [FANFARE_BW_DEX_RATING_1]         = { BW_SEQ_ME_HYOUKA1,         200 },
    [FANFARE_BW_DEX_RATING_2]         = { BW_SEQ_ME_HYOUKA2,         180 },
    [FANFARE_BW_DEX_RATING_3]         = { BW_SEQ_ME_HYOUKA3,         220 },
    [FANFARE_BW_DEX_RATING_4]         = { BW_SEQ_ME_HYOUKA4,         210 },
    [FANFARE_BW_DEX_RATING_5]         = { BW_SEQ_ME_HYOUKA5,         210 },
    [FANFARE_BW_DEX_RATING_6]         = { BW_SEQ_ME_HYOUKA6,         370 },
    [FANFARE_BW_XTRANSCEIVER]         = { BW_SEQ_ME_CALL,            210 },
    [FANFARE_BW_OBTAIN_ITEM]          = { BW_SEQ_ME_ITEM,            160 },
    [FANFARE_BW_EVOLVED]              = { BW_SEQ_ME_SHINKAOME,       240 },
    [FANFARE_BW_OBTAIN_BADGE]         = { BW_SEQ_ME_BADGE,           340 },
    [FANFARE_BW_OBTAIN_TMHM]          = { BW_SEQ_ME_WAZA,            220 },
    [FANFARE_BW_MOVE_DELETED]         = { BW_SEQ_ME_WASURE,          180 },
    [FANFARE_BW_LIGHT_DARK_STONE]     = { BW_SEQ_ME_ITEM_MYSTERY,    195 },
    [FANFARE_BW_RECEIVE_EGG]          = { BW_SEQ_ME_TAMAGO_GET,      155 },
    [FANFARE_BW_OBTAIN_B_POINTS]      = { BW_SEQ_ME_BPGET,           250 },
    [FANFARE_BW_ACCESSORY]            = { BW_SEQ_ME_ACCE,            160 },
    [FANFARE_BW_FEELING_CHECK_START]  = { BW_SEQ_ME_AISHOU_KIDOU,    162 },
    [FANFARE_BW_FEELING_CHECK_3]      = { BW_SEQ_ME_AISHOU_H,        244 },
    [FANFARE_BW_FEELING_CHECK_2]      = { BW_SEQ_ME_AISHOU_M,        200 },
    [FANFARE_BW_FEELING_CHECK_1]      = { BW_SEQ_ME_AISHOU_L,        120 },
    [FANFARE_BW_MISSION_START]        = { BW_SEQ_ME_MISSION_START,   280 },
    [FANFARE_BW_MISSION_CLEAR]        = { BW_SEQ_ME_MISSION_CLEAR,   270 },
    [FANFARE_BW_MISSION_FAILED]       = { BW_SEQ_ME_MISSION_FAILED,  180 },
    [FANFARE_BW_OBTAIN_BERRY]         = { BW_SEQ_ME_KINOMI,          120 },
};

void InitMapMusic(void)
{
    gDisableMusic = FALSE;
    ResetMapMusic();
}

void MapMusicMain(void)
{
    switch (sMapMusicState)
    {
    case 0:
        break;
    case 1:
        sMapMusicState = 2;
        PlayBGM(sCurrentMapMusic);
        break;
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        if (IsBGMStopped())
        {
            sNextMapMusic = 0;
            sMapMusicState = 0;
        }
        break;
    case 6:
        if (IsBGMStopped() && IsFanfareTaskInactive())
        {
            sCurrentMapMusic = sNextMapMusic;
            sNextMapMusic = 0;
            sMapMusicState = 2;
            PlayBGM(sCurrentMapMusic);
        }
        break;
    case 7:
        if (IsBGMStopped() && IsFanfareTaskInactive())
        {
            FadeInNewBGM(sNextMapMusic, sMapMusicFadeInSpeed);
            sCurrentMapMusic = sNextMapMusic;
            sNextMapMusic = 0;
            sMapMusicState = 2;
            sMapMusicFadeInSpeed = 0;
        }
        break;
    }
}

void ResetMapMusic(void)
{
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 0;
    sMapMusicFadeInSpeed = 0;
}

u16 GetCurrentMapMusic(void)
{
    return sCurrentMapMusic;
}

void PlayNewMapMusic(u16 songNum)
{
    sCurrentMapMusic = songNum;
    sNextMapMusic = 0;
    sMapMusicState = 1;
}

void StopMapMusic(void)
{
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 1;
}

void FadeOutMapMusic(u8 speed)
{
    if (IsNotWaitingForBGMStop())
        FadeOutBGM(speed);
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 5;
}

void FadeOutAndPlayNewMapMusic(u16 songNum, u8 speed)
{
    FadeOutMapMusic(speed);
    sCurrentMapMusic = 0;
    sNextMapMusic = songNum;
    sMapMusicState = 6;
}

void FadeOutAndFadeInNewMapMusic(u16 songNum, u8 fadeOutSpeed, u8 fadeInSpeed)
{
    FadeOutMapMusic(fadeOutSpeed);
    sCurrentMapMusic = 0;
    sNextMapMusic = songNum;
    sMapMusicState = 7;
    sMapMusicFadeInSpeed = fadeInSpeed;
}

void FadeInNewMapMusic(u16 songNum, u8 speed)
{
    FadeInNewBGM(songNum, speed);
    sCurrentMapMusic = songNum;
    sNextMapMusic = 0;
    sMapMusicState = 2;
    sMapMusicFadeInSpeed = 0;
}

bool8 IsNotWaitingForBGMStop(void)
{
    if (sMapMusicState == 6)
        return FALSE;
    if (sMapMusicState == 5)
        return FALSE;
    if (sMapMusicState == 7)
        return FALSE;
    return TRUE;
}

void PlayFanfareByFanfareNum(u8 fanfareNum)
{
    u16 songNum;
    m4aMPlayStop(&gMPlayInfo_BGM);
    songNum = sFanfares[fanfareNum].songNum;
    sFanfareCounter = sFanfares[fanfareNum].duration;
    m4aSongNumStart(songNum);
}

bool8 WaitFanfare(bool8 stop)
{
    if (sFanfareCounter)
    {
        sFanfareCounter--;
        return FALSE;
    }
    else
    {
        if (!stop)
            m4aMPlayContinue(&gMPlayInfo_BGM);
        else
            m4aSongNumStart(MUS_DUMMY);

        return TRUE;
    }
}

// Unused
void StopFanfareByFanfareNum(u8 fanfareNum)
{
    m4aSongNumStop(sFanfares[fanfareNum].songNum);
}

void PlayFanfare(u16 songNum)
{
    s32 i;
    for (i = 0; (u32)i < ARRAY_COUNT(sFanfares); i++)
    {
        if (sFanfares[i].songNum == songNum)
        {
            PlayFanfareByFanfareNum(i);
            CreateFanfareTask();
            return;
        }
    }

    // songNum is not in sFanfares
    // Play first fanfare in table instead
    PlayFanfareByFanfareNum(0);
    CreateFanfareTask();
}

bool8 IsFanfareTaskInactive(void)
{
    if (FuncIsActiveTask(Task_Fanfare) == TRUE)
        return FALSE;
    return TRUE;
}

static void Task_Fanfare(u8 taskId)
{
    if (sFanfareCounter)
    {
        sFanfareCounter--;
    }
    else
    {
        m4aMPlayContinue(&gMPlayInfo_BGM);
        DestroyTask(taskId);
    }
}

static void CreateFanfareTask(void)
{
    if (FuncIsActiveTask(Task_Fanfare) != TRUE)
        CreateTask(Task_Fanfare, 80);
}

void FadeInNewBGM(u16 songNum, u8 speed)
{
    if (gDisableMusic)
        songNum = 0;
    if (songNum == MUS_NONE)
        songNum = 0;
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_BGM);
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0);
    m4aSongNumStop(songNum);
    m4aMPlayFadeIn(&gMPlayInfo_BGM, speed);
}

void FadeOutBGMTemporarily(u8 speed)
{
    m4aMPlayFadeOutTemporarily(&gMPlayInfo_BGM, speed);
}

bool8 IsBGMPausedOrStopped(void)
{
    if (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
        return TRUE;
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return TRUE;
    return FALSE;
}

void FadeInBGM(u8 speed)
{
    m4aMPlayFadeIn(&gMPlayInfo_BGM, speed);
}

void FadeOutBGM(u8 speed)
{
    m4aMPlayFadeOut(&gMPlayInfo_BGM, speed);
}

bool8 IsBGMStopped(void)
{
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return TRUE;
    return FALSE;
}

void PlayCry_Normal(u16 species, s8 pan)
{
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
    PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, CRY_MODE_NORMAL);
    gPokemonCryBGMDuckingCounter = 2;
    RestoreBGMVolumeAfterPokemonCry();
}

void PlayCry_NormalNoDucking(u16 species, s8 pan, s8 volume, u8 priority)
{
    PlayCryInternal(species, pan, volume, priority, CRY_MODE_NORMAL);
}

// Assuming it's not CRY_MODE_DOUBLES, this is equivalent to PlayCry_Normal except it allows other modes.
void PlayCry_ByMode(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
        gPokemonCryBGMDuckingCounter = 2;
        RestoreBGMVolumeAfterPokemonCry();
    }
}

// Used when releasing multiple Pokémon at once in battle.
void PlayCry_ReleaseDouble(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
}

// Duck the BGM but don't restore it. Not present in R/S
void PlayCry_DuckNoRestore(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
        gPokemonCryBGMDuckingCounter = 2;
    }
}

void PlayCry_Script(u16 species, u8 mode)
{
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
    PlayCryInternal(species, 0, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    gPokemonCryBGMDuckingCounter = 2;
    RestoreBGMVolumeAfterPokemonCry();
}

void PlayCryInternal(u16 species, s8 pan, s8 volume, u8 priority, u8 mode)
{
    bool32 reverse;
    u32 release;
    u32 length;
    u32 pitch;
    u32 chorus;

    // Set default values
    // May be overridden depending on mode.
    length = 140;
    reverse = FALSE;
    release = 0;
    pitch = 15360;
    chorus = 0;

    switch (mode)
    {
    case CRY_MODE_NORMAL:
        break;
    case CRY_MODE_DOUBLES:
        length = 20;
        release = 225;
        break;
    case CRY_MODE_ENCOUNTER:
        release = 225;
        pitch = 15600;
        chorus = 20;
        volume = 90;
        break;
    case CRY_MODE_HIGH_PITCH:
        length = 50;
        release = 200;
        pitch = 15800;
        chorus = 20;
        volume = 90;
        break;
    case CRY_MODE_ECHO_START:
        length = 25;
        reverse = TRUE;
        release = 100;
        pitch = 15600;
        chorus = 192;
        volume = 90;
        break;
    case CRY_MODE_FAINT:
        release = 200;
        pitch = 14440;
        break;
    case CRY_MODE_ECHO_END:
        release = 220;
        pitch = 15555;
        chorus = 192;
        volume = 70;
        break;
    case CRY_MODE_ROAR_1:
        length = 10;
        release = 100;
        pitch = 14848;
        break;
    case CRY_MODE_ROAR_2:
        length = 60;
        release = 225;
        pitch = 15616;
        break;
    case CRY_MODE_GROWL_1:
        length = 15;
        reverse = TRUE;
        release = 125;
        pitch = 15200;
        break;
    case CRY_MODE_GROWL_2:
        length = 100;
        release = 225;
        pitch = 15200;
        break;
    case CRY_MODE_WEAK_DOUBLES:
        length = 20;
        release = 225;
        // fallthrough
    case CRY_MODE_WEAK:
        pitch = 15000;
        break;
    }

    SetPokemonCryVolume(volume);
    SetPokemonCryPanpot(pan);
    SetPokemonCryPitch(pitch);
    SetPokemonCryLength(length);
    SetPokemonCryProgress(0);
    SetPokemonCryRelease(release);
    SetPokemonCryChorus(chorus);
    SetPokemonCryPriority(priority);

    species--;
    gMPlay_PokemonCry = SetPokemonCryTone(reverse ? &gCryTable_Reverse[species] : &gCryTable[species]);
}

bool8 IsCryFinished(void)
{
    if (FuncIsActiveTask(Task_DuckBGMForPokemonCry) == TRUE)
    {
        return FALSE;
    }
    else
    {
        ClearPokemonCrySongs();
        return TRUE;
    }
}

void StopCryAndClearCrySongs(void)
{
    m4aMPlayStop(gMPlay_PokemonCry);
    ClearPokemonCrySongs();
}

void StopCry(void)
{
    m4aMPlayStop(gMPlay_PokemonCry);
}

bool8 IsCryPlayingOrClearCrySongs(void)
{
    if (IsPokemonCryPlaying(gMPlay_PokemonCry))
    {
        return TRUE;
    }
    else
    {
        ClearPokemonCrySongs();
        return FALSE;
    }
}

bool8 IsCryPlaying(void)
{
    if (IsPokemonCryPlaying(gMPlay_PokemonCry))
        return TRUE;
    else
        return FALSE;
}

static void Task_DuckBGMForPokemonCry(u8 taskId)
{
    if (gPokemonCryBGMDuckingCounter)
    {
        gPokemonCryBGMDuckingCounter--;
        return;
    }

    if (!IsPokemonCryPlaying(gMPlay_PokemonCry))
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 256);
        DestroyTask(taskId);
    }
}

static void RestoreBGMVolumeAfterPokemonCry(void)
{
    if (FuncIsActiveTask(Task_DuckBGMForPokemonCry) != TRUE)
        CreateTask(Task_DuckBGMForPokemonCry, 80);
}

void PlayBGM(u16 songNum)
{
    if (gDisableMusic)
        songNum = 0;
    if (songNum == MUS_NONE)
        songNum = 0;
    m4aSongNumStart(songNum);
}

void PlaySE(u16 songNum)
{
    m4aSongNumStart(songNum);
}

void PlaySE12WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE1);
    m4aMPlayImmInit(&gMPlayInfo_SE2);
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

void PlaySE1WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE1);
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
}

void PlaySE2WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE2);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

void SE12PanpotControl(s8 pan)
{
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

bool8 IsSEPlaying(void)
{
    if ((gMPlayInfo_SE1.status & MUSICPLAYER_STATUS_PAUSE) && (gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_PAUSE))
        return FALSE;
    if (!(gMPlayInfo_SE1.status & MUSICPLAYER_STATUS_TRACK) && !(gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}

bool8 IsBGMPlaying(void)
{
    if (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
        return FALSE;
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}

bool8 IsSpecialSEPlaying(void)
{
    if (gMPlayInfo_SE3.status & MUSICPLAYER_STATUS_PAUSE)
        return FALSE;
    if (!(gMPlayInfo_SE3.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}
