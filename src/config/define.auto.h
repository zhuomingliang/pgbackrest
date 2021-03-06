/***********************************************************************************************************************************
Command and Option Definition

Automatically generated by Build.pm -- do not modify directly.
***********************************************************************************************************************************/
#ifndef CONFIG_DEFINE_AUTO_H
#define CONFIG_DEFINE_AUTO_H

/***********************************************************************************************************************************
Command define enum
***********************************************************************************************************************************/
typedef enum
{
    cfgDefCmdArchiveGet,
    cfgDefCmdArchivePush,
    cfgDefCmdBackup,
    cfgDefCmdCheck,
    cfgDefCmdExpire,
    cfgDefCmdHelp,
    cfgDefCmdInfo,
    cfgDefCmdLocal,
    cfgDefCmdRemote,
    cfgDefCmdRestore,
    cfgDefCmdStanzaCreate,
    cfgDefCmdStanzaDelete,
    cfgDefCmdStanzaUpgrade,
    cfgDefCmdStart,
    cfgDefCmdStop,
    cfgDefCmdVersion,
} ConfigDefineCommand;

/***********************************************************************************************************************************
Option type define enum
***********************************************************************************************************************************/
typedef enum
{
    cfgDefOptTypeBoolean,
    cfgDefOptTypeFloat,
    cfgDefOptTypeHash,
    cfgDefOptTypeInteger,
    cfgDefOptTypeList,
    cfgDefOptTypeString,
} ConfigDefineOptionType;

/***********************************************************************************************************************************
Option define enum
***********************************************************************************************************************************/
typedef enum
{
    cfgDefOptArchiveAsync,
    cfgDefOptArchiveCheck,
    cfgDefOptArchiveCopy,
    cfgDefOptArchiveQueueMax,
    cfgDefOptArchiveTimeout,
    cfgDefOptBackupStandby,
    cfgDefOptBufferSize,
    cfgDefOptChecksumPage,
    cfgDefOptCmdSsh,
    cfgDefOptCommand,
    cfgDefOptCompress,
    cfgDefOptCompressLevel,
    cfgDefOptCompressLevelNetwork,
    cfgDefOptConfig,
    cfgDefOptDbInclude,
    cfgDefOptDbTimeout,
    cfgDefOptDelta,
    cfgDefOptForce,
    cfgDefOptHostId,
    cfgDefOptLinkAll,
    cfgDefOptLinkMap,
    cfgDefOptLockPath,
    cfgDefOptLogLevelConsole,
    cfgDefOptLogLevelFile,
    cfgDefOptLogLevelStderr,
    cfgDefOptLogPath,
    cfgDefOptLogTimestamp,
    cfgDefOptManifestSaveThreshold,
    cfgDefOptNeutralUmask,
    cfgDefOptOnline,
    cfgDefOptOutput,
    cfgDefOptPerlBin,
    cfgDefOptPerlOption,
    cfgDefOptPgHost,
    cfgDefOptPgHostCmd,
    cfgDefOptPgHostConfig,
    cfgDefOptPgHostPort,
    cfgDefOptPgHostUser,
    cfgDefOptPgPath,
    cfgDefOptPgPort,
    cfgDefOptPgSocketPath,
    cfgDefOptProcess,
    cfgDefOptProcessMax,
    cfgDefOptProtocolTimeout,
    cfgDefOptRecoveryOption,
    cfgDefOptRepoCipherPass,
    cfgDefOptRepoCipherType,
    cfgDefOptRepoHardlink,
    cfgDefOptRepoHost,
    cfgDefOptRepoHostCmd,
    cfgDefOptRepoHostConfig,
    cfgDefOptRepoHostPort,
    cfgDefOptRepoHostUser,
    cfgDefOptRepoPath,
    cfgDefOptRepoS3Bucket,
    cfgDefOptRepoS3CaFile,
    cfgDefOptRepoS3CaPath,
    cfgDefOptRepoS3Endpoint,
    cfgDefOptRepoS3Host,
    cfgDefOptRepoS3Key,
    cfgDefOptRepoS3KeySecret,
    cfgDefOptRepoS3Region,
    cfgDefOptRepoS3VerifySsl,
    cfgDefOptRepoType,
    cfgDefOptResume,
    cfgDefOptRetentionArchive,
    cfgDefOptRetentionArchiveType,
    cfgDefOptRetentionDiff,
    cfgDefOptRetentionFull,
    cfgDefOptSet,
    cfgDefOptSpoolPath,
    cfgDefOptStanza,
    cfgDefOptStartFast,
    cfgDefOptStopAuto,
    cfgDefOptTablespaceMap,
    cfgDefOptTablespaceMapAll,
    cfgDefOptTarget,
    cfgDefOptTargetAction,
    cfgDefOptTargetExclusive,
    cfgDefOptTargetTimeline,
    cfgDefOptTest,
    cfgDefOptTestDelay,
    cfgDefOptTestPoint,
    cfgDefOptType,
} ConfigDefineOption;

#endif
