#pragma once

#include <string>

#include "Engine/Audio/AudioSystem.hpp"

const std::string AUDIO_ANTICIPATION_SRC = std::string( "Data/Audio/Anticipation.mp3" );
const std::string AUDIO_ATTRACT_MUSIC_SRC = std::string( "Data/Audio/AttractMusic.mp3" );
const std::string AUDIO_PLAY_MUSIC_SRC = std::string( "Data/Audio/GameplayMusic.mp3" );

const std::string AUDIO_PAUSE_ACTIVATE_SRC = std::string( "Data/Audio/Pause.mp3" );
const std::string AUDIO_PAUSE_DEACTIVATE_SRC = std::string( "Data/Audio/Unpause.mp3" );

const std::string AUDIO_PLAYER_SHOOT_SRC = std::string( "Data/Audio/PlayerShootNormal.ogg" );
const std::string AUDIO_PLAYER_HIT_SRC = std::string( "Data/Audio/PlayerHit.wav" );
const std::string AUDIO_PLAYER_DIED_SRC = std::string( "Data/Audio/PlayerDied.wav" );

const std::string AUDIO_ENEMY_SHOOT_SRC = std::string( "Data/Audio/EnemyShoot.wav" );
const std::string AUDIO_ENEMY_HIT_SRC = std::string( "Data/Audio/EnemyHit.wav" );
const std::string AUDIO_ENEMY_DIED_SRC = std::string( "Data/Audio/EnemyDied.wav" );

extern SoundID AUDIO_ANTICIPATION;
extern SoundID AUDIO_ATTRACT_MUSIC;
extern SoundID AUDIO_PLAY_MUSIC;

extern SoundID AUDIO_PAUSE_ACTIVATE;
extern SoundID AUDIO_PAUSE_DEACTIVATE;

extern SoundID AUDIO_PLAYER_SHOOT;
extern SoundID AUDIO_PLAYER_HIT;
extern SoundID AUDIO_PLAYER_DIED;

extern SoundID AUDIO_ENEMY_SHOOT;
extern SoundID AUDIO_ENEMY_HIT;
extern SoundID AUDIO_ENEMY_DIED;