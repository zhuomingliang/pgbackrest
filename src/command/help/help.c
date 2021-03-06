/***********************************************************************************************************************************
Help Command
***********************************************************************************************************************************/
#include <string.h>
#include <unistd.h>

#include "common/memContext.h"
#include "common/type.h"
#include "config/config.h"
#include "config/define.h"
#include "version.h"

/***********************************************************************************************************************************
Define the console width - use a fixed with of 80 since this should be safe on virtually all consoles
***********************************************************************************************************************************/
#define CONSOLE_WIDTH                                               80

/***********************************************************************************************************************************
Helper function for helpRender() to make output look good on a console
***********************************************************************************************************************************/
static String *
helpRenderText(const String *text, int indent, bool indentFirst, int length)
{
    String *result = strNew("");

    // Split the text into paragraphs
    StringList *lineList = strLstNewSplitZ(text, "\n");

    // Iterate through each paragraph and split the lines according to the line length
    for (unsigned int lineIdx = 0; lineIdx < strLstSize(lineList); lineIdx++)
    {
        // Add LF if there is already content
        if (strSize(result) != 0)
            strCat(result, "\n");

        // Split the paragraph into lines that don't exceed the line length
        StringList *partList = strLstNewSplitSizeZ(strLstGet(lineList, lineIdx), " ", length - indent);

        for (unsigned int partIdx = 0; partIdx < strLstSize(partList); partIdx++)
        {
            // Indent when required
            if (partIdx != 0 || indentFirst)
            {
                if (partIdx != 0)
                    strCat(result, "\n");

                if (strSize(strLstGet(partList, partIdx)))
                    strCatFmt(result, "%*s", indent, "");
            }

            // Add the line
            strCat(result, strPtr(strLstGet(partList, partIdx)));
        }
    }

    return result;
}

/***********************************************************************************************************************************
Helper function for helpRender() to output values as strings
***********************************************************************************************************************************/
static String *
helpRenderValue(const Variant *value)
{
    String *result = NULL;

    if (value != NULL)
    {
        if (varType(value) == varTypeBool)
        {
            if (varBool(value))
                result = strNew("y");
            else
                result = strNew("n");
        }
        else
            result = varStrForce(value);
    }

    return result;
}

/***********************************************************************************************************************************
Render help to a string
***********************************************************************************************************************************/
static String *
helpRender()
{
    String *result = strNew(PGBACKREST_NAME " " PGBACKREST_VERSION);

    MEM_CONTEXT_TEMP_BEGIN()
    {
        // Message for more help when it is available
        String *more = NULL;

        // Display general help
        if (cfgCommand() == cfgCmdHelp || cfgCommand() == cfgCmdNone)
        {
            strCat(
                result,
                " - General help\n"
                "\n"
                "Usage:\n"
                "    " PGBACKREST_BIN " [options] [command]\n"
                "\n"
                "Commands:\n");

            // Find size of longest command name
            size_t commandSizeMax = 0;

            for (ConfigCommand commandId = 0; commandId < CFG_COMMAND_TOTAL; commandId++)
            {
                if (commandId == cfgCmdNone)
                    continue;

                if (strlen(cfgCommandName(commandId)) > commandSizeMax)
                    commandSizeMax = strlen(cfgCommandName(commandId));
            }

            // Output help for each command
            for (ConfigCommand commandId = 0; commandId < CFG_COMMAND_TOTAL; commandId++)
            {
                if (commandId == cfgCmdNone)
                    continue;

                const char *helpSummary = cfgDefCommandHelpSummary(cfgCommandDefIdFromId(commandId));

                if (helpSummary != NULL)
                {
                    strCatFmt(
                        result, "    %s%*s%s\n", cfgCommandName(commandId),
                        (int)(commandSizeMax - strlen(cfgCommandName(commandId)) + 2), "",
                        strPtr(helpRenderText(strNew(helpSummary), commandSizeMax + 6, false, CONSOLE_WIDTH)));
                }
            }

            // Construct message for more help
            more = strNew("[command]");
        }
        else
        {
            ConfigCommand commandId = cfgCommand();
            ConfigDefineCommand commandDefId = cfgCommandDefIdFromId(commandId);
            const char *commandName = cfgCommandName(commandId);

            // Output command part of title
            strCatFmt(result, " - '%s' command", commandName);

            // If no additional params then this is command help
            if (strLstSize(cfgCommandParam()) == 0)
            {
                // Output command summary and description
                strCatFmt(
                    result,
                    " help\n"
                    "\n"
                    "%s\n"
                    "\n"
                    "%s\n",
                    strPtr(helpRenderText(strNew(cfgDefCommandHelpSummary(commandDefId)), 0, true, CONSOLE_WIDTH)),
                    strPtr(helpRenderText(strNew(cfgDefCommandHelpDescription(commandDefId)), 0, true, CONSOLE_WIDTH)));

                // Construct key/value of sections and options
                KeyValue *optionKv = kvNew();
                size_t optionSizeMax = 0;

                for (unsigned int optionDefId = 0; optionDefId < cfgDefOptionTotal(); optionDefId++)
                {
                    if (cfgDefOptionValid(commandDefId, optionDefId) && !cfgDefOptionInternal(commandDefId, optionDefId))
                    {
                        String *section = NULL;

                        if (cfgDefOptionHelpSection(optionDefId) != NULL)
                            section = strNew(cfgDefOptionHelpSection(optionDefId));

                        if (section == NULL ||
                            (!strEqZ(section, "general") && !strEqZ(section, "log") && !strEqZ(section, "repository") &&
                             !strEqZ(section, "stanza") && !strEqZ(section, "expire")))
                        {
                            section = strNew("command");
                        }

                        kvAdd(optionKv, varNewStr(section), varNewInt((int)optionDefId));

                        if (strlen(cfgDefOptionName(optionDefId)) > optionSizeMax)
                            optionSizeMax = strlen(cfgDefOptionName(optionDefId));
                    }
                }

                // Output sections
                StringList *sectionList = strLstSort(strLstNewVarLst(kvKeyList(optionKv)), sortOrderAsc);

                for (unsigned int sectionIdx = 0; sectionIdx < strLstSize(sectionList); sectionIdx++)
                {
                    const String *section = strLstGet(sectionList, sectionIdx);

                    strCatFmt(result, "\n%s Options:\n\n", strPtr(strFirstUpper(strDup(section))));

                    // Output options
                    VariantList *optionList = kvGetList(optionKv, varNewStr(section));

                    for (unsigned int optionIdx = 0; optionIdx < varLstSize(optionList); optionIdx++)
                    {
                        ConfigDefineOption optionDefId = varInt(varLstGet(optionList, optionIdx));
                        ConfigOption optionId = cfgOptionIdFromDefId(optionDefId, 0);

                        // Get option summary
                        String *summary = strFirstLower(strNewN(
                            cfgDefOptionHelpSummary(commandDefId, optionDefId),
                            strlen(cfgDefOptionHelpSummary(commandDefId, optionDefId)) - 1));

                        // Ouput current and default values if they exist
                        String *defaultValue = helpRenderValue(cfgOptionDefault(optionId));
                        String *value = NULL;

                        if (cfgOptionSource(optionId) != cfgSourceDefault)
                            value = helpRenderValue(cfgOption(optionId));

                        if (value != NULL || defaultValue != NULL)
                        {
                            strCat(summary, " [");

                            if (value != NULL)
                                strCatFmt(summary, "current=%s", strPtr(value));

                            if (defaultValue != NULL)
                            {
                                if (value != NULL)
                                    strCat(summary, ", ");

                                strCatFmt(summary, "default=%s", strPtr(defaultValue));
                            }

                            strCat(summary, "]");
                        }

                        // Output option help
                        strCatFmt(
                            result, "  --%s%*s%s\n",
                            cfgDefOptionName(optionDefId), (int)(optionSizeMax - strlen(cfgDefOptionName(optionDefId)) + 2), "",
                            strPtr(helpRenderText(summary, optionSizeMax + 6, false, CONSOLE_WIDTH)));
                    }
                }

                // Construct message for more help if there are options
                if (optionSizeMax > 0)
                    more = strNewFmt("%s [option]", commandName);
            }
            // Else option help for the specified command
            else
            {
                // Make sure only one option was specified
                if (strLstSize(cfgCommandParam()) > 1)
                    THROW(ParamInvalidError, "only one option allowed for option help");

                // Ensure the option is valid
                const char *optionName = strPtr(strLstGet(cfgCommandParam(), 0));
                ConfigOption optionId = cfgOptionId(optionName);

                if (cfgOptionId(optionName) == -1)
                {
                    if (cfgDefOptionId(optionName) != -1)
                        optionId = cfgOptionIdFromDefId(cfgDefOptionId(optionName), 0);
                    else
                        THROW(OptionInvalidError, "option '%s' is not valid for command '%s'", optionName, commandName);
                }

                // Output option summary and description
                ConfigDefineOption optionDefId = cfgOptionDefIdFromId(optionId);

                strCatFmt(
                    result,
                    " - '%s' option help\n"
                    "\n"
                    "%s\n"
                    "\n"
                    "%s\n",
                    optionName,
                    strPtr(helpRenderText(strNew(cfgDefOptionHelpSummary(commandDefId, optionDefId)), 0, true, CONSOLE_WIDTH)),
                    strPtr(helpRenderText(strNew(cfgDefOptionHelpDescription(commandDefId, optionDefId)), 0, true, CONSOLE_WIDTH)));

                // Ouput current and default values if they exist
                String *defaultValue = helpRenderValue(cfgOptionDefault(optionId));
                String *value = NULL;

                if (cfgOptionSource(optionId) != cfgSourceDefault)
                    value = helpRenderValue(cfgOption(optionId));

                if (value != NULL || defaultValue != NULL)
                {
                    strCat(result, "\n");

                    if (value != NULL)
                        strCatFmt(result, "current: %s\n", strPtr(value));

                    if (defaultValue != NULL)
                        strCatFmt(result, "default: %s\n", strPtr(defaultValue));
                }

                // Output alternate names (call them deprecated so the user will know not to use them)
                if (cfgDefOptionHelpNameAlt(optionDefId))
                {
                    strCat(result, "\ndeprecated name");

                    if (cfgDefOptionHelpNameAltValueTotal(optionDefId) > 1)
                        strCat(result, "s");                        // {uncovered - no option has more than one alt name}

                    strCat(result, ": ");

                    for (int nameAltIdx = 0; nameAltIdx < cfgDefOptionHelpNameAltValueTotal(optionDefId); nameAltIdx++)
                    {
                        if (nameAltIdx != 0)
                            strCat(result, ", ");                   // {uncovered - no option has more than one alt name}

                        strCat(result, cfgDefOptionHelpNameAltValue(optionDefId, nameAltIdx));
                    }

                    strCat(result, "\n");
                }
            }
        }

        // If there is more help available output a message to let the user know
        if (more != NULL)
            strCatFmt(result, "\nUse '" PGBACKREST_BIN " help %s' for more information.\n", strPtr(more));
    }
    MEM_CONTEXT_TEMP_END();

    return result;
}

/***********************************************************************************************************************************
Render help and output to stdout
***********************************************************************************************************************************/
void
cmdHelp()
{
    MEM_CONTEXT_TEMP_BEGIN()
    {
        String *help = helpRender();

        THROW_ON_SYS_ERROR(
            write(STDOUT_FILENO, strPtr(help), strSize(help)) != (int)strSize(help), FileWriteError,
            "unable to write help to stdout");
    }
    MEM_CONTEXT_TEMP_END();
}
