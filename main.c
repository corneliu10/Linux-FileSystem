#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_LEN 25

typedef enum type {
    File = 0,
    Folder
} FileType;

typedef struct TreeNode {
    char name[PATH_LEN];
    int nrChilds;
    FileType type;

    struct TreeNode **childs;
    struct TreeNode *father;
} TreeNode;

TreeNode *root;

TreeNode* createTreeNode(TreeNode *father) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->nrChilds = 0;
    node->type = File;
    node->childs = NULL;
    node->father = father;
    strcpy(node->name, "");

    return node;
}

char** splitLine(const char *line, char delim[], int *nrWords) {
    char **words = (char**) malloc(sizeof(char*));
    char *ptr = strtok(line, delim);

	while(ptr != NULL)
	{
	    words = (char**) realloc(words, (*nrWords + 1) * sizeof(char*));
	    words[*nrWords] = malloc(25 * sizeof(char));
	    strcpy(words[(*nrWords)++], ptr);
	    // printf("'%s'\n", ptr);
		ptr = strtok(NULL, delim);
	}

	return words;
}

TreeNode* findPath(TreeNode* node, char path[]) {
    if (strlen(path) == 0)
        return node;

    if (path[0] == '/') {
        node = root;

        if (strlen(path) == 0)
            return root;
    }

    int nrWords = 0;
    char delim[] = "/";
    char** words = splitLine(path, delim, &nrWords);

    for (int j = 0; j < nrWords; j++) {
        int ok = 0;
        for (int i = 0; i < node->nrChilds; ++i)
            if (strcmp(node->childs[i]->name, words[j]) == 0) {
                ok = 1;
                node = node->childs[i];
                break;
            }

        if (!ok) return NULL;
    }

    return node;
}

void addPath(TreeNode *root, char dir[PATH_LEN], FileType type) {
    if (findPath(root, dir) != NULL) {
        printf("%s: already exists\n", dir);
        return;
    }

    TreeNode **newChilds = (TreeNode**) realloc(root->childs, (root->nrChilds + 1) * sizeof(TreeNode*));
    if (newChilds != NULL) {
        root->childs = newChilds;
        root->childs[root->nrChilds] = createTreeNode(root);
        root->childs[root->nrChilds]->type = type;
        strcpy(root->childs[root->nrChilds++]->name, dir);
    }
}

void copyPath(TreeNode *root, char path[], char destPath[]) {
    TreeNode *src = findPath(root, path);
    if (src == NULL) {
        printf("%s: No such file or directory\n", path);
        return;
    }

    TreeNode *dest = findPath(root, destPath);
    if (dest == NULL) {
        printf("%s: No such file or directory\n", destPath);
        return;
    }

    addPath(dest, src->name, src->type);
}

void movePath(TreeNode *root, char path[], char destPath[]) {
    copyPath(root, path, destPath);

    TreeNode *src = findPath(root, path);
    if (src == NULL) {
        printf("%s: No such file or directory\n", path);
        return;
    }

    removePath(src->father, src->name, File);
}

void removePath(TreeNode *root, char dir[PATH_LEN], FileType type) {
    TreeNode *node = findPath(root, dir);
    if (node == NULL) {
        printf("%s: No such file or directory\n", dir);
        return;
    }

    root = node->father;
    for (int i = 0; i < root->nrChilds; ++i)
        if (strcmp(root->childs[i]->name, dir) == 0) {
            if (type == Folder) {
                if (root->childs[i]->nrChilds > 0) {
                    printf("%s: Directory not empty\n", dir);
                    return;
                }
                if (root->childs[i]->type == File) {
                    printf("%s: Not a directory\n", dir);
                    return;
                }
            }

            root->childs[i] = NULL; // TODO: fix memory leak
            for (int j = i; j < root->nrChilds - 1; ++j)
                root->childs[j] = root->childs[j+1];

            free(root->childs[root->nrChilds]);
            root->nrChilds--;
            return;
        }

}

void printTree(TreeNode *root) {
    pwd(root);
    printf("\n");
    for (int i = 0; i < root->nrChilds; ++i) {
        printTree(root->childs[i]);
    }
}

TreeNode* generateTree(TreeNode *root) {
    if (root == NULL) {
        root = createTreeNode(NULL);
        strcpy(root->name, "/");
    }

    addPath(root, "ana", Folder);
    addPath(root, "abc", Folder);
    addPath(root, "a.out", File);
    addPath(root, "man", File);
    addPath(root, "manual", Folder);

    TreeNode* node = findPath(root, "abc");
    addPath(node, "a", Folder);
    addPath(node, "b", Folder);
    addPath(node, "c", Folder);

    node = findPath(root, "manual");
    addPath(node, "page1", File);
    addPath(node, "page2", File);
    addPath(node, "page3", File);

    node = findPath(root, "ana");
    addPath(node, "are", Folder);

    node = findPath(node, "are");
    addPath(node, "mere", Folder);

    return root;
}

char* readLine() {
    size_t bufsize = 64;
    char* buffer = malloc(bufsize * sizeof(char));

    fgets(buffer, bufsize, stdin);
    /* remove newline, if present */
    int i = strlen(buffer) - 1;
    if( buffer[i] == '\n')
        buffer[i] = '\0';

    return buffer;
}

TreeNode* cd(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords < 2) {
        printf("cd: missing operand");
    } else if (nrWords > 2) {
        printf("cd: too many arguments");
    } else {
        if (strcmp(words[1], "..") == 0) {
            if (currDir->father != NULL) {
                currDir = currDir->father;
            }
        } else if (strcmp(words[1], "/") == 0) {
            return root;
        } else if (strcmp(words[1], "./") == 0) {
            return currDir;
        } else {
            TreeNode* node = findPath(currDir, words[1]);
            if (node != NULL) {
                if (node->type == File) {
                    printf("%s: Not a directory\n", words[1]);
                } else {
                    currDir = node;
                }
            } else {
                printf("%s: No such file or directory\n", words[1]);
            }
        }
    }

    return currDir;
}

int createFiles(TreeNode* currDir, char** words, int nrWords, FileType type) {
    if (nrWords <= 1) {
        return 0;
    } else {
        for (int i = 1; i < nrWords; ++i)
            addPath(currDir, words[i], type);

        return 1;
    }
}

int deleteFiles(TreeNode* currDir, char** words, int nrWords, FileType type) {
    if (nrWords <= 1) {
        return 0;
    } else {
        for (int i = 1; i < nrWords; ++i)
            removePath(currDir, words[i], type);

        return 1;
    }
}

int copyFiles(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords <= 2) {
        return 0;
    } else {
        char* destPath = words[nrWords - 1];
        for (int i = 1; i < nrWords - 1; ++i)
            copyPath(currDir, words[i], destPath);

        return 1;
    }
}

int moveFiles(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords <= 2) {
        return 0;
    } else {
        char* destPath = words[nrWords - 1];
        for (int i = 1; i < nrWords - 1; ++i)
            movePath(currDir, words[i], destPath);

        return 1;
    }
}

int removeFiles(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords <= 1) {
        return 0;
    } else {
        for (int i = 1; i < nrWords; ++i)
            removePath(currDir, words[i], File);

        return 1;
    }
}

TreeNode* executeCommand(TreeNode* currDir, char** words, int nrWords) {
    if (strcmp(words[0], "pwd") == 0) {
        pwd(currDir);
    }
    else if (strcmp(words[0], "ls") == 0) {
        ls(currDir, words, nrWords);
    } else if (strcmp(words[0], "cd") == 0) {
        TreeNode* node = cd(currDir, words, nrWords);
        currDir = node;
    } else if (strcmp(words[0], "mkdir") == 0) {
        if (!createFiles(currDir, words, nrWords, Folder))
            printf("mkdir: missing operand");
    } else if (strcmp(words[0], "touch") == 0) {
        if (!createFiles(currDir, words, nrWords, File))
            printf("touch: missing operand");
    }  else if (strcmp(words[0], "rmdir") == 0) {
        if (!deleteFiles(currDir, words, nrWords, Folder))
            printf("rmdir: missing operand");
    }  else if (strcmp(words[0], "cp") == 0) {
        if (!copyFiles(currDir, words, nrWords))
            printf("cp: missing operand");
    }  else if (strcmp(words[0], "mv") == 0) {
        if (!moveFiles(currDir, words, nrWords))
            printf("mv: missing operand");
    }  else if (strcmp(words[0], "rm") == 0) {
        if (!removeFiles(currDir, words, nrWords))
            printf("rm: missing operand");
    }  else if (strcmp(words[0], "print") == 0) {
        printTree(root);
    } else if (nrWords > 0) {
        printf("%s: command not found", words[0]);
    }

    printf("\n");
    return currDir;
}

void run() {
    root = generateTree(NULL);
    TreeNode *currDir = root;
    char *line;
    char delim[] = " ";
    char** words = NULL;
    int nrWords = 0;

    while(1) {
        line = readLine();
        words = splitLine(line, delim, &nrWords);
        currDir = executeCommand(currDir, words, nrWords);

        free(words);
        nrWords = 0;
    }
}

void pwd(TreeNode* currDir) {
    if (currDir->father != NULL)
        pwd(currDir->father);

    printf("%s",currDir);
    if (currDir->type == Folder)
        printf("/");
}

void ls(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords > 3) {
        printf("ls: too many arguments");
    } else {
        int explicitFormat = 0;
        if (nrWords >= 2) {
            if (strcmp(words[1], "-F") == 0)
                explicitFormat = 1;
            else
                currDir = findPath(currDir, words[1]);
        }

        if (nrWords == 3) {
            currDir = findPath(currDir, words[2]);
            if (currDir == NULL) {
                printf("%s: No such file or directory", words[2]);
                return;
            }
        }

        for (int i = 0; i < currDir->nrChilds; ++i)
            if (explicitFormat) {
                if (currDir->childs[i]->type == Folder)
                    printf("%s/ ", currDir->childs[i]);
                else
                    printf("%s* ", currDir->childs[i]);
            } else {
                printf("%s ", currDir->childs[i]);
            }
    }
}

int main()
{
    run();
}
