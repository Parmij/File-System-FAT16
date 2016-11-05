#include <iostream.h>
#include <stdio.h>
#include <dos.h>
#include<string.h>
#include<ctype.h>

struct boot {
    char ignore[11];
    unsigned int bytesPerSector;
    unsigned char sectorsPerCluster;
    unsigned int numberOfReservedSecotrs;
    unsigned char numberOfFATs;
    unsigned int maxiumNumberOfRootDirectoryEntries;
    unsigned int totalSectorCount;
    char ignore2;
    unsigned int sectorsPerFAT;
    unsigned int sectorsPerTrack;
    unsigned int nuberOfHeads;
    char stuffing[484];
};

struct FCB {
    unsigned char filename[8];
    unsigned char extension[3];
    unsigned char attribute;
    unsigned int reserved;
    unsigned int creationTime;
    unsigned int creationDate;
    unsigned int lastAccessedDate;
    unsigned int ignore;
    unsigned int lastWriteTime;
    unsigned int lastWriteDate;
    unsigned int firstLogicalCluster;
    unsigned long fileSize;
};

void printStr(char *str) {
    for (int i = 0; i < strlen(str); i++) {
	cout << str[i];
    }
}

int compareStr(char *str1, char *str2) {
    //	cout<<strlen(str1)<<"  "<<strlen(str2)<<endl;
    if (strlen(str1) > strlen(str2) || strlen(str1) < strlen(str2))
	return 0;
    for (int i = 0; i < strlen(str1); i++) {
	if (str1[i] != str2[i])
	    return 0;
    }
    return 1;
}

FCB root[16];
boot bootSector;
unsigned int *fat;

void dirCommand() {
    cout << "Directory of D:\\" << endl << endl;
    int counter = 0;
    int stopRead = 0;
    for (int v = 0; v < 32; v++) {
	if (stopRead == 1)
	    break;
	if (absread(3, 1, 217 + v, root) == 0) {
	    for (int i = 0; i < 16; i++) {
		if (root[i].filename[0] == 0) {
		    stopRead = 1;
		    break;
		}
		if (root[i].filename[0] == '?' || root[i].attribute == 0x0F)// || root[i].filename[0] == 0xE5)
		    continue;
		for (int k = 0; k < 8; k++)
		    printf("%c", root[i].filename[k]);
		cout << " ";
		int isDir = 0;
		counter++;
		for (int kk = 0; kk < 3; kk++) {
		    if (root[i].extension[0] == ' ') {
			isDir = 1;
			cout << "   ";
			break;
		    }
		    printf("%c", root[i].extension[kk]);
		}
		cout << "   ";
		if (isDir == 1)
		    cout << "<DIR>";
		else
		    printf("%5ld", root[i].fileSize);
		// printf("  ? %f ?\n",root[i].creationDate);
		cout << "   " << root[i].lastWriteTime<<endl;
	 //	cout << "   " << root[i].creationTime << endl;
	    }
	}
    }
    cout << counter << " files and folders" << endl;
}

void typeCommand() {
    char fileName[12];
    cin>>fileName;
    //cout<<fileName<<endl;
    char temp[9];
    char extTemp[4];
    char temp2[9];
    char extTemp2[4];
    int fileFound = 0;
    for (int N = 0; N < 9; N++) {
	temp[N] = '\0';
	temp2[N] = '\0';
    }
    for (N = 0; N < 4; N++) {
	extTemp[N] = '\0';
	extTemp2[N] = '\0';
    }
    int dotIndex = -1;
    for (int i = 0; i < 8; i++) {
	if (fileName[i] == '.') {
	    dotIndex = i + 1;
	    break;
	}
	temp[i] = fileName[i];
    }
    if (dotIndex != -1) {
	for (i = 0; i < 3; i++) {
	    extTemp[i] = fileName[dotIndex];
	    dotIndex++;
	}
    }
    int stopRead = 0;
    for (int v = 0; v < 32; v++) {
	if (stopRead == 1)
	    break;
        if (absread(3, 1, 217 + v, root) == 0) {
            for (int j = 0; j < 16; j++) {
                if (root[j].filename[0] == 0) {
                    stopRead = 1;
                    break;
                }
                if (root[j].filename[0] == '?' || root[j].attribute == 0x0F)
                    continue;
                if (root[j].attribute & 0x10)
                    continue;
                for (int N = 0; N < 9; N++) {
                    temp2[N] = '\0';
                }
                for (N = 0; N < 4; N++) {
                    extTemp2[N] = '\0';
                }
                for (int k = 0; k < 8; k++) {
                    if (root[j].filename[k] != ' ')
                        temp2[k] = tolower(root[j].filename[k]);
		}
                for (k = 0; k < 3; k++) {
                    if (root[j].extension[k] != ' ')
                        extTemp2[k] = tolower(root[j].extension[k]);
                }
                if (compareStr(temp, temp2) == 1 && compareStr(extTemp, extTemp2) == 1) {
                    fileFound = 1;
                    int FLC = root[j].firstLogicalCluster;
                    char buffer[2048];
                    absread(3, 4, 217 + (512 / 16) + 4 * FLC - 8, buffer);
                    int size = 0;
                    for (int l = 0; l < 2048; l++) {
                        if (size == root[j].fileSize)
                            break;
                        size++;
                        printf("%c", buffer[l]);
                    }
                    while (1) {
                        FLC = fat[FLC];
                        if (FLC == 0xfff8 || FLC == 0xffff || FLC == 0)
			    break;
                        absread(3, 4, 217 + (512 / 16) + 4 * FLC - 8, buffer);
                        for (int l = 0; l < 2048; l++) {
                            if (size >= root[j].fileSize)
                                break;
                            size++;
                            printf("%c", buffer[l]);
                        }
                    }
                    break;

                }
            }
        }
    }
    if (fileFound == 0)
        cout << "No file found with name " << fileName << endl;
}

void dirPath(char *path) {

    unsigned char tempFileName[9];
    int FLC;
    int numberSlash = 0;
    int endReadRootDirectory = 0;
    int dirFound = 0;
    for (int n = 0; n < 9; n++) {
        tempFileName[n] = '\0';
    }
    int j = 0;
    for (int i = 0; i < strlen(path); i++) {

        if (path[i] == '\\' || i == strlen(path) - 1) {
            numberSlash++;
            if (i == strlen(path) - 1)
                tempFileName[j++] = path[i];
            //cout << tempFileName << endl;
            if (numberSlash <= 1) {
                for (int v = 0; v < 32; v++) {
                    if (endReadRootDirectory)
			break;

                    if (absread(3, 1, 217 + v, root) == 0) {
                        for (int k = 0; k < 16; k++) {
                            if (root[k].filename[0] == 0) {
                                endReadRootDirectory = 1;
                                break;
                            }
                            if (root[k].filename[0] == '?' || root[k].attribute == 0x0F)
                                continue;
                            if (!(root[k].attribute & 0x10))
                                continue;
                            unsigned char temp2 [9];
                            for (int ll = 0; ll < 9; ll++)
                                temp2[ll] = '\0';
                            for (int l = 0; l < 8; l++) {
                                if (root[k].filename[l] != ' ') {
                                    temp2[l] = tolower(root[k].filename[l]);
                                }
                            }
			    if (compareStr(temp2, tempFileName)) {
                                dirFound = 1;
                                // cout << "Dir found" << endl;
                                FLC = root[k].firstLogicalCluster;
                            }
                        }
                    }
                }
            } else {
                //if (FLC == 0xffff || FLC == 0xFFF8)
                //  break;
                dirFound = 0;
                //	cout << FLC << endl;
                absread(3, 1, 217 + (512 / 16) + 4 * FLC - 8, root);
                for (int mm = 0; mm < 16; mm++) {
                    if (root[mm].filename[0] == 0)
                        break;
                    if (root[mm].filename[0] == '?' || root[mm].attribute == 0x0F)
                        continue;
                    if (!(root[mm].attribute & 0x10))
			continue;
                    unsigned char temp2 [9];
                    for (int ll = 0; ll < 9; ll++)
                        temp2[ll] = '\0';
                    for (int l = 0; l < 8; l++) {
                        if (root[mm].filename[l] != ' ') {
                            temp2[l] = tolower(root[mm].filename[l]);
                        }
                    }
                    if (compareStr(temp2, tempFileName)) {
                        dirFound = 1;
                        //	cout<<temp2<<"  "<<tempFileName<<endl;
                        FLC = root[mm].firstLogicalCluster;
                    }
                }
            }
            //----------------------------
            if (!dirFound) {
                cout << "Dir not found" << endl;
                break;
	    }
            for (j = 0; j < 9; j++) {
                tempFileName[j] = '\0';
            }
            j = 0;
            continue;
        }
        tempFileName[j++] = path[i];
    }
    if (dirFound) {
        //cout << "Dir is ok" << endl;
        if (absread(3, 1, 217 + (512 / 16) + 4 * FLC - 8, root) == 0) {
            for (int i = 0; i < 16; i++) {

                if (root[i].filename[0] == 0)
                    break;

                if (root[i].filename[0] == '?' || root[i].attribute == 0x0F)
                    continue;

		for (int j = 0; j < 8; j++) {
                    printf("%c", root[i].filename[j]);

                }
                if (root[i].attribute & 0x10) {
                    cout << "    ";
                    cout << "    ";
                    cout << "<DIR>\n";
                } else {

                    for (int k = 0; k < 3; k++) {
                        printf("%c", root[i].extension[k]);
                    }
                    cout << "     ";
                    cout << root[i].fileSize << endl;
                }
            }
            cout << endl;
        }
    }
}

void unDeleteCommand(char *delFileName) {
    cout << delFileName << endl;
    int fileFounded = 0;
    int stopRead = 0;
    for (int v = 0; v < 32; v++) {
        if (stopRead || fileFounded)
            break;
        if (absread(3, 1, 217 + v, root) == 0) {
            for (int i = 0; i < 16; i++) {
                if (root[i].filename[0] == 0) {
                    stopRead = 1;
                    break;
                }
                //if (root[i].attribute & 0x10)
                //	continue;
                if (root[i].filename[0] == 0xE5) {
                    // cout<<root[i].filename<<endl;
                    char tempDel1[8];
		    char tempExt[3];
		    memset(tempDel1, '\0', sizeof tempDel1);
		    memset(tempExt, '\0', sizeof tempExt);
		    for (int k = 1; k < 8; k++) {
			if (root[i].filename[k] != ' ')
			    tempDel1[k] = tolower(root[i].filename[k]);
		    }
		    //                    for (int k = 0; k < 3; i++) {
		    //                        if(root[i].extension[k]!=' ')
		    //                            tempDel1[k] = root[i].extension[k];
		    //                    }
		    char tempDel2[8];
		    for (int kk = 1; kk < 8; kk++) {
			tempDel2[kk] = tolower(delFileName[kk]);
		    }
		    tempDel1[0] = ' ';
		    tempDel2[0] = ' ';
		    // cout<<tempDel2<<"  "<<tempDel1<<"   fcdfd"<<endl;
		    if (compareStr(tempDel2, tempDel1)) {
			//cout<<"Found"<<endl;
			fileFounded = 1;
			cout << "File found\nDo you want to restor it (Y,N)?\n";
			char option;
			cin>>option;
			if (option == 'y' || option == 'Y') {
			    root[i].filename[0] = toupper(delFileName[0]);
			    int b1 = abswrite(3, 1, 217 + v, &root);
			    fat[root[i].firstLogicalCluster] = -1;
			    int b2 = 1; //abswrite(3, bootSector.sectorsPerFAT, 1, &fat);
			    //cout<<"b1= "<<b1;
			    // cout<<"\nb2= "<<b2<<endl;
			    absread(3, bootSector.sectorsPerFAT, 1, fat);
			    if (!b1 && !b2)
				cout << "Done successfuly" << endl;
			}
			break;
		    }
		}
	    }
	}
    }
    if (!fileFounded)
	cout << "File Not Found" << endl;
}


int searchFileBoool = 0;

void searchFileRec(char *fn, char *fe, int FLC, char *currentPath) {
    //cout<<"REC "<<fn<<"  "<<FLC<<endl;
    if(FLC == 0 || FLC == 26980)
	return;
    char temp2[9];
    char extTemp2[4];
    FCB tempFCB[16];
    if(absread(3, 1, 217 + (512 / 16) + 4 * FLC - 8, tempFCB)==0){
    for (int i = 0; i < 16; i++) {
	if (tempFCB[i].filename[0] == 0)
	    break;
	if (tempFCB[i].filename[0] == '?' || tempFCB[i].attribute == 0x0F)
	    continue;
	for (int N = 0; N < 9; N++) {
	    temp2[N] = '\0';
	}
	for (N = 0; N < 4; N++) {
	    extTemp2[N] = '\0';
	}
	for (int k = 0; k < 8; k++) {
	    if (tempFCB[i].filename[k] != ' ')
		temp2[k] = tolower(tempFCB[i].filename[k]);
	}
	if ((tempFCB[i].attribute & 0x10) && tempFCB[i].filename[0]!='.') {
	    char inpath[40];
	    memset(inpath,'\0',sizeof inpath);
	    strcat(inpath,currentPath);
	    //strcat(inpath,"\\");
	    strcat(inpath,temp2);
	   // cout<<tempFCB[i].firstLogicalCluster<<endl;
	    searchFileRec(fn, fe, tempFCB[i].firstLogicalCluster, inpath);
	    continue;
	}
	for (k = 0; k < 3; k++) {
	    if (tempFCB[i].extension[k] != ' ')
		extTemp2[k] = tolower(tempFCB[i].extension[k]);
	}
       //	cout<<temp2<<"."<<extTemp2<<" "<<fn<<"."<<fe<<endl;
	if (compareStr(fn, temp2) == 1 && compareStr(fe, extTemp2) == 1) {
	    cout << "File found at " <<currentPath<< endl;
	    searchFileBoool = 1;
	}
    }
  }
}

void searchFile(char *fn) {
    char temp[9];
    char extTemp[4];
    char temp2[9];
    char extTemp2[4];
    for (int N = 0; N < 9; N++) {
	temp[N] = '\0';
	temp2[N] = '\0';
    }
    for (N = 0; N < 4; N++) {
	extTemp[N] = '\0';
	extTemp2[N] = '\0';
    }
    int dotIndex = -1;
    for (int i = 0; i < 8; i++) {
	if (fn[i] == '.') {
	    dotIndex = i + 1;
	    break;
	}
	temp[i] = fn[i];
    }
    if (dotIndex != -1) {
	for (i = 0; i < 3; i++) {
	    extTemp[i] = fn[dotIndex];
	    dotIndex++;
	}
    }
    int stopRead = 0;
    for (int v = 0; v < 32; v++) {
	if (stopRead == 1)
	    break;
	if (absread(3, 1, 217 + v, root) == 0) {
	    for (int i = 0; i < 16; i++) {
		if (root[i].filename[0] == 0) {
		    stopRead = 1;
		    break;
		}
		if (root[i].filename[0] == '?' || root[i].attribute == 0x0F || root[i].filename[0] == 0xE5)
		    continue;
		for (int N = 0; N < 9; N++) {
		    temp2[N] = '\0';
		}
		for (N = 0; N < 4; N++) {
		    extTemp2[N] = '\0';
		}
		for (int k = 0; k < 8; k++) {
		    if (root[i].filename[k] != ' ')
			temp2[k] = tolower(root[i].filename[k]);
		}
		if (root[i].attribute & 0x10) {
		    char tempPath[40];
		    memset(tempPath,'\0',sizeof tempPath);
		    strcat(tempPath,"D:\\");
		    strcat(tempPath,temp2);
		    strcat(tempPath,"\\");
		   // cout<<root[i].firstLogicalCluster<<endl;
		    searchFileRec(temp, extTemp, root[i].firstLogicalCluster, tempPath);
		    continue;
		}
		for (k = 0; k < 3; k++) {
		    if (root[i].extension[k] != ' ')
			extTemp2[k] = tolower(root[i].extension[k]);
		}
		if (compareStr(temp, temp2) == 1 && compareStr(extTemp, extTemp2) == 1) {
		    cout << "File found at D:\\" << endl;
		    searchFileBoool = 1;
		}
	    }
	}
    }
}

void main() {
    absread(3, 1, 0, &bootSector);
    fat = new unsigned int [256 * bootSector.sectorsPerFAT];
    absread(3, bootSector.sectorsPerFAT, 1, fat);
    char input[10];
    cin>>input;
    while (1) {
        if (compareStr(input, "exit") == 1) {
            cout << "Program terminated" << endl;
            break;
        } else if (compareStr(input, "dir") == 1) {
            while (1) {
                char c = cin.get();
                if (c == ' ')
                    continue;
                else if (c == '\n') {
                    dirCommand();
                    break;
                } else {
                    char path[40];
                    memset(path, '\0', sizeof (path));
                    char temp2[40];
                    path[0] = c;
                    cin>>temp2;
                    //strcat(path,c);
                    strcat(path, temp2);
                    //cout << path << endl;
                    dirPath(path);
                    break;
                }
            }

        } else if (compareStr(input, "type") == 1) {
            typeCommand();
        } else if (compareStr(input, "undelete") == 1) {
            char Fname[11];
            cin>>Fname;
            unDeleteCommand(Fname);

        } else if (compareStr(input, "search") == 1) {
            searchFileBoool = 0;
            char fn[12];
            cin>>fn;
            searchFile(fn);
            if(!searchFileBoool)
                cout<<"NO file found with name "<<fn<<endl;
        } else {
            cout << input << " unrecognized command press type help for more information\n";
        }
        cin>>input;

    }
    delete[] fat;
}