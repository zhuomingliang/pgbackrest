#!/usr/bin/perl
####################################################################################################################################
# File.pl - Unit Tests for BackRest::File
####################################################################################################################################

####################################################################################################################################
# Perl includes
####################################################################################################################################
use strict;
use warnings;
use english;

use Carp;
use File::Basename;
use Cwd 'abs_path';

use lib dirname($0) . "/..";
use pg_backrest_file;
use pg_backrest_utility;

use Exporter qw(import);
our @EXPORT = qw(BackRestTestFile);

sub BackRestTestFile
{
    my $strLockPath = dirname(abs_path($0)) . "/lock";
    my $strTestPath = dirname(abs_path($0)) . "/test";
    my $iRun;

    my $strStanza = "db";
    my $strCommand = "/Users/dsteele/pg_backrest/bin/pg_backrest_command.pl";
    my $strHost = "127.0.0.1";
    my $strUser = getpwuid($<);
    my $strGroup = getgrgid($();
    
    umask(0);
    
#    print "user = ${strUser}, group = ${strGroup}";

#    log_level_set(TRACE, TRACE);

    # Test manifest()
    $iRun = 0;

    print "\ntest File->manifest()\n";

    # Create the test data
    system("rm -rf test");

    system("mkdir -m 750 ${strTestPath}") == 0 or confess "Unable to create test directory";
    system("mkdir -m 750 ${strTestPath}/sub1") == 0 or confess "Unable to create test directory";
    system("mkdir -m 750 ${strTestPath}/sub1/sub2") == 0 or confess "Unable to create test directory";

    system("echo 'TESTDATA' > ${strTestPath}/test.txt");
    utime(1111111111, 1111111111, "${strTestPath}/test.txt");
    system("chmod 1640 ${strTestPath}/test.txt");

    system("echo 'TESTDATA_' > ${strTestPath}/sub1/test-sub1.txt");
    utime(1111111112, 1111111112, "${strTestPath}/sub1/test-sub1.txt");
    system("chmod 0640 ${strTestPath}/sub1/test-sub1.txt");

    system("echo 'TESTDATA__' > ${strTestPath}/sub1/sub2/test-sub2.txt");
    utime(1111111113, 1111111113, "${strTestPath}/sub1/sub2/test-sub2.txt");
    system("chmod 0646 ${strTestPath}/sub1/test-sub1.txt");

    system("ln ${strTestPath}/test.txt ${strTestPath}/sub1/test-hardlink.txt");
    system("ln ${strTestPath}/test.txt ${strTestPath}/sub1/sub2/test-hardlink.txt");

    system("ln -s .. ${strTestPath}/sub1/test");
    system("chmod 0700 ${strTestPath}/sub1/test");
    system("ln -s ../.. ${strTestPath}/sub1/sub2/test");
    system("chmod 0750 ${strTestPath}/sub1/sub2/test");

    my $strManifestCompare =
        ".,d,${strUser},${strGroup},0750,,,,\n" .
        "sub1,d,${strUser},${strGroup},0750,,,,\n" .
        "sub1/sub2,d,${strUser},${strGroup},0750,,,,\n" .
        "sub1/sub2/test,l,${strUser},${strGroup},,,,,../..\n" .
        "sub1/sub2/test-hardlink.txt,f,${strUser},${strGroup},1640,1111111111,0,9,\n" .
        "sub1/sub2/test-sub2.txt,f,${strUser},${strGroup},0666,1111111113,0,11,\n" .
        "sub1/test,l,${strUser},${strGroup},,,,,..\n" .
        "sub1/test-hardlink.txt,f,${strUser},${strGroup},1640,1111111111,0,9,\n" .
        "sub1/test-sub1.txt,f,${strUser},${strGroup},0646,1111111112,0,10,\n" .
        "test.txt,f,${strUser},${strGroup},1640,1111111111,0,9,";

    for (my $bRemote = 0; $bRemote <= 1; $bRemote++)
    {
        my $oFile = pg_backrest_file->new
        (
            strStanza => $strStanza,
            bNoCompression => true,
            strCommand => $strCommand,
            strBackupClusterPath => ${strTestPath},
            strBackupPath => ${strTestPath},
            strBackupHost => $bRemote ? $strHost : undef,
            strBackupUser => $bRemote ? $strUser : undef
        );

        for (my $bError = 0; $bError <= 1; $bError++)
        {
            $iRun++;

            print "run ${iRun} - " .
                  "remote $bRemote, error $bError\n";
            
            my $strPath = $strTestPath;

            if ($bError)
            {
                $strPath .= "-error";
            }

            # Execute in eval in case of error
            eval
            {
                my %oManifestHash;
                $oFile->manifest(PATH_BACKUP_ABSOLUTE, $strPath, \%oManifestHash);

                my $strManifest;

                foreach my $strName (sort(keys $oManifestHash{name}))
                {
                    if (!defined($strManifest))
                    {
                        $strManifest = "";
                    }
                    else
                    {
                        $strManifest .= "\n";
                    }

                    if (defined($oManifestHash{name}{"${strName}"}{inode}))
                    {
                        $oManifestHash{name}{"${strName}"}{inode} = 0;
                    }

                    $strManifest .= 
                        "${strName}," .
                        $oManifestHash{name}{"${strName}"}{type} . "," .
                        (defined($oManifestHash{name}{"${strName}"}{user}) ?
                            $oManifestHash{name}{"${strName}"}{user} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{group}) ?
                            $oManifestHash{name}{"${strName}"}{group} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{permission}) ?
                            $oManifestHash{name}{"${strName}"}{permission} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{modification_time}) ?
                            $oManifestHash{name}{"${strName}"}{modification_time} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{inode}) ?
                            $oManifestHash{name}{"${strName}"}{inode} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{size}) ?
                            $oManifestHash{name}{"${strName}"}{size} : "") . "," .
                        (defined($oManifestHash{name}{"${strName}"}{link_destination}) ?
                            $oManifestHash{name}{"${strName}"}{link_destination} : "");
                }

                if ($strManifest ne $strManifestCompare)
                {
                    confess "manifest is not equal:\n\n${strManifest}\n\ncompare:\n\n${strManifestCompare}\n\n";
                }
            };

            if ($@ && !$bError)
            {
                confess "error raised: " . $@ . "\n";
            }
        }
    }

    return;

    # Test list()
    $iRun = 0;

    print "\ntest File->list()\n";

    for (my $bRemote = 0; $bRemote <= 1; $bRemote++)
    {
        my $oFile = pg_backrest_file->new
        (
            strStanza => $strStanza,
            bNoCompression => true,
            strCommand => $strCommand,
            strBackupClusterPath => ${strTestPath},
            strBackupPath => ${strTestPath},
            strBackupHost => $bRemote ? $strHost : undef,
            strBackupUser => $bRemote ? $strUser : undef
        );

        # Loop through exists
        for (my $bSort = 0; $bSort <= 1; $bSort++)
        {
            my $strSort = $bSort ? undef : "reverse";

            # Loop through expression
            for (my $iExpression = 0; $iExpression <= 2; $iExpression++)
            {
                my $strExpression;

                # Expression tha returns results
                if ($iExpression == 1)
                {
                    $strExpression = "^test2\\..*\$";
                }
                # Expression that does not return results
                elsif ($iExpression == 2)
                {
                    $strExpression = "^du\$";
                }

                # Loop through exists
                for (my $bExists = 0; $bExists <= 1; $bExists++)
                {
                    $iRun++;

                    print "run ${iRun} - " .
                          "remote $bRemote, exists $bExists, " .
                          "expression " . (defined($strExpression) ? $strExpression : "[undef]") . ", " .
                          "sort " . (defined($strSort) ? $strSort : "[undef]") . "\n";

                    my $strPath = "${strTestPath}";

                    # Drop the old test directory and create a new one
                    system("rm -rf test");

                    if ($bExists)
                    {
                        system("mkdir test") == 0 or confess "Unable to create test directory";
                        system("echo 'TESTDATA' > ${strPath}/test.txt");
                        system("echo 'TESTDATA2' > ${strPath}/test2.txt");
                    }

                    my @stryFileCompare = split(/\n/, "test.txt\ntest2.txt");

                    # Execute in eval in case of error
                    eval
                    {
                        my @stryFileList = $oFile->list(PATH_BACKUP_ABSOLUTE, $strPath, $strExpression, $strSort);

                        if (defined($strExpression))
                        {
                            @stryFileCompare = grep(/$strExpression/i, @stryFileCompare);
                        }

                        if (defined($strSort))
                        {
                            @stryFileCompare = sort {$b cmp $a} @stryFileCompare;
                        }

                        my $strFileList = sprintf("@stryFileList");
                        my $strFileCompare = sprintf("@stryFileCompare");

                        if ($strFileList ne $strFileCompare)
                        {
                            confess "list (${strFileList})[" . @stryFileList . 
                                    "] does not match compare (${strFileCompare})[" . @stryFileCompare . "]";
                        }
                    };

                    if ($@ && $bExists)
                    {
                        confess "error raised: " . $@ . "\n";
                    }
                }
            }
        }
    }

    # Test remove()
    $iRun = 0;
    print "test File->remove()\n";

    for (my $bRemote = 0; $bRemote <= 1; $bRemote++)
    {
        my $strHost = $bRemote ? "127.0.0.1" : undef;
        my $strUser = $bRemote ? "dsteele" : undef;

        my $oFile = pg_backrest_file->new
        (
            strStanza => $strStanza,
            bNoCompression => true,
            strCommand => ${strCommand},
            strBackupClusterPath => ${strTestPath},
            strBackupPath => ${strTestPath},
            strBackupHost => $bRemote ? $strHost : undef,
            strBackupUser => $bRemote ? $strUser : undef
        );

        # Loop through exists
        for (my $bExists = 0; $bExists <= 1; $bExists++)
        {
            # Loop through temp
            for (my $bTemp = 0; $bTemp <= 1; $bTemp++)
            {
                # Loop through ignore missing
                for (my $bIgnoreMissing = 0; $bIgnoreMissing <= 1; $bIgnoreMissing++)
                {
                    $iRun++;

                    print "run ${iRun} - " .
                          "remote ${bRemote}, exists ${bExists}, temp ${bTemp}, ignore missing ${bIgnoreMissing}\n";

                    # Drop the old test directory and create a new one
                    system("rm -rf test");
                    system("mkdir test") == 0 or confess "Unable to create test directory";

                    my $strFile = "${strTestPath}/test.txt";
                    
                    if ($bExists)
                    {
                        system("echo 'TESTDATA' > ${strFile}" . ($bTemp ? ".backrest.tmp" : ""));
                    }

                    # Execute in eval in case of error
                    eval
                    {
                        if ($oFile->remove(PATH_BACKUP_ABSOLUTE, $strFile, $bTemp, $bIgnoreMissing) != $bExists)
                        {
                            confess "hash did not match expected";
                        }
                    };

                    if ($@ && $bExists)
                    {
                        confess "error raised: " . $@ . "\n";
                    }

                    if (-e ($strFile . ($bTemp ? ".backrest.tmp" : "")))
                    {
                        confess "file still exists";
                    }
                }
            }
        }
    }

    # Test hash()
    $iRun = 0;
    
    print "\ntest File->hash()\n";

    for (my $bRemote = 0; $bRemote <= 1; $bRemote++)
    {
        my $oFile = pg_backrest_file->new
        (
            strStanza => $strStanza,
            bNoCompression => true,
            strCommand => $strCommand,
            strBackupClusterPath => ${strTestPath},
            strBackupPath => ${strTestPath},
            strBackupHost => $bRemote ? $strHost : undef,
            strBackupUser => $bRemote ? $strUser : undef
        );

        # Loop through exists
        for (my $bExists = 0; $bExists <= 1; $bExists++)
        {
            $iRun++;

            print "run ${iRun} - " .
                  "remote $bRemote, exists $bExists\n";

            # Drop the old test directory and create a new one
            system("rm -rf test");
            system("mkdir test") == 0 or confess "Unable to create test directory";

            my $strFile = "${strTestPath}/test.txt";

            if ($bExists)
            {
                system("echo 'TESTDATA' > ${strFile}");
            }

            # Execute in eval in case of error
            eval
            {
                if ($oFile->hash(PATH_BACKUP_ABSOLUTE, $strFile) ne '06364afe79d801433188262478a76d19777ef351')
                {
                    confess "bExists is set to ${bExists}, but exists() returned " . !$bExists;
                }
            };

            if ($@)
            {
                if ($bExists)
                {
                    confess "error raised: " . $@ . "\n";
                }
            }
        }
    }

    # Test exists()
    $iRun = 0;
    
    print "\ntest File->exists()\n";

    for (my $bRemote = 0; $bRemote <= 1; $bRemote++)
    {
        my $oFile = pg_backrest_file->new
        (
            strStanza => $strStanza,
            bNoCompression => true,
            strCommand => $strCommand,
            strBackupClusterPath => ${strTestPath},
            strBackupPath => ${strTestPath},
            strBackupHost => $bRemote ? $strHost : undef,
            strBackupUser => $bRemote ? $strUser : undef
        );

        # Loop through exists
        for (my $bExists = 0; $bExists <= 1; $bExists++)
        {
            $iRun++;

            print "run ${iRun} - " .
                  "remote $bRemote, exists $bExists\n";

            # Drop the old test directory and create a new one
            system("rm -rf test");
            system("mkdir test") == 0 or confess "Unable to create test directory";

            my $strFile = "${strTestPath}/test.txt";

            if ($bExists)
            {
                system("echo 'TESTDATA' > ${strFile}");
            }

            # Execute in eval in case of error
            eval
            {
                if ($oFile->exists(PATH_BACKUP_ABSOLUTE, $strFile) != $bExists)
                {
                    confess "bExists is set to ${bExists}, but exists() returned " . !$bExists;
                }
            };
            
            if ($@)
            {
                confess "error raised: " . $@ . "\n";
            }
        }
    }

    return;

    # Test copy()
    $iRun = 0;

    system("rm -rf lock");
    system("mkdir -p lock") == 0 or confess "Unable to create lock directory";

    for (my $bBackupRemote = 0; $bBackupRemote <= 1; $bBackupRemote++)
    {
        my $strBackupHost = $bBackupRemote ? "127.0.0.1" : undef;
        my $strBackupUser = $bBackupRemote ? "dsteele" : undef;

        # Loop through source compression
        for (my $bDbRemote = 0; $bDbRemote <= 1; $bDbRemote++)
        {
            my $strDbHost = $bDbRemote ? "127.0.0.1" : undef;
            my $strDbUser = $bDbRemote ? "dsteele" : undef;

            # Loop through destination compression
            for (my $bDestinationCompressed = 0; $bDestinationCompressed <= 1; $bDestinationCompressed++)
            {
                my $oFile = pg_backrest_file->new
                (
                    strStanza => "db",
                    bNoCompression => !$bDestinationCompressed,
                    strBackupClusterPath => undef,
                    strBackupPath => ${strTestPath},
                    strBackupHost => $strBackupHost,
                    strBackupUser => $strBackupUser,
                    strDbHost => $strDbHost,
                    strDbUser => $strDbUser,
                    strCommandCompress => "gzip --stdout %file%",
                    strCommandDecompress => "gzip -dc %file%",
                    strLockPath => dirname($0) . "/test/lock"
                );

                for (my $bSourceCompressed = 0; $bSourceCompressed <= 1; $bSourceCompressed++)
                {
                    for (my $bSourcePathType = 0; $bSourcePathType <= 1; $bSourcePathType++)
                    {
                        my $strSourcePath = $bSourcePathType ? PATH_DB_ABSOLUTE : PATH_BACKUP_ABSOLUTE;

                        for (my $bDestinationPathType = 0; $bDestinationPathType <= 1; $bDestinationPathType++)
                        {
                            my $strDestinationPath = $bDestinationPathType ? PATH_DB_ABSOLUTE : PATH_BACKUP_ABSOLUTE;

                            for (my $bError = 0; $bError <= 1; $bError++)
                            {
                                for (my $bConfessError = 0; $bConfessError <= 1; $bConfessError++)
                                {
                                    $iRun++;
                                
                                    print "run ${iRun} - " .
                                          "srcpth ${strSourcePath}, bkprmt $bBackupRemote, srccmp $bSourceCompressed, " .
                                          "dstpth ${strDestinationPath}, dbrmt $bDbRemote, dstcmp $bDestinationCompressed, " .
                                          "error $bError, confess_error $bConfessError\n";

                                    # Drop the old test directory and create a new one
                                    system("rm -rf test");
                                    system("mkdir -p test/lock") == 0 or confess "Unable to create the test directory";

                                    my $strSourceFile = "${strTestPath}/test-source.txt";
                                    my $strDestinationFile = "${strTestPath}/test-destination.txt";

                                    # Create the compressed or uncompressed test file
                                    if ($bSourceCompressed)
                                    {
                                        $strSourceFile .= ".gz";
                                        system("echo 'TESTDATA' | gzip > ${strSourceFile}");
                                    }
                                    else
                                    {
                                        system("echo 'TESTDATA' > ${strSourceFile}");
                                    }
                                    # Create the file object based on current values

                                    if ($bError)
                                    {
                                        $strSourceFile .= ".error";
                                    }

                                    # Run file copy in an eval block because some errors are expected
                                    my $bReturn;

                                    eval
                                    {
                                        $bReturn = $oFile->file_copy($strSourcePath, $strSourceFile,
                                                                     $strDestinationPath, $strDestinationFile,
                                                                     undef, undef, undef, undef,
                                                                     $bConfessError);
                                    };

                                    # Check for errors after copy
                                    if ($@)
                                    {
                                        # Different remote and destination with different path types should error
                                        if ($bBackupRemote && $bDbRemote && ($strSourcePath ne $strDestinationPath))
                                        {
                                            print "    different source and remote for same path not supported\n";
                                            next;
                                        }
                                        # If the error was intentional, then also continue
                                        elsif ($bError)
                                        {
                                            my $strError = $oFile->error_get();

                                            if (!defined($strError) || ($strError eq ''))
                                            {
                                                confess 'no error message returned';
                                            }

                                            print "    error raised: ${strError}\n";
                                            next;
                                        }
                                        # Else this is an unexpected error
                                        else
                                        {
                                            confess $@;
                                        }
                                    }
                                    elsif ($bError)
                                    {
                                        if ($bConfessError)
                                        {
                                            confess "Value was returned instead of exception thrown when confess error is true";
                                        }
                                        else
                                        {
                                            if ($bReturn)
                                            {
                                                confess "true was returned when an error was generated";
                                            }
                                            else
                                            {
                                                my $strError = $oFile->error_get();
                                                
                                                if (!defined($strError) || ($strError eq ''))
                                                {
                                                    confess 'no error message returned';
                                                }

                                                print "    error returned: ${strError}\n";
                                                next;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (!$bReturn)
                                        {
                                            confess "error was returned when no error generated";
                                        }
                                        
                                        print "    true was returned\n";
                                    }

                                    # Check for errors after copy
                                    if ($bDestinationCompressed)
                                    {
                                        $strDestinationFile .= ".gz";
                                    }

                                    unless (-e $strDestinationFile)
                                    {
                                        confess "could not find destination file ${strDestinationFile}";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
