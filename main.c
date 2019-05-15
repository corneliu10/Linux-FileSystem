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

FILE *in, *out, *err;
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

char** splitLine(char *line, char delim[], int *nrWords) {
    char copyLine[256];
    strcpy(copyLine, line);
    char **words = (char**) malloc(sizeof(char*));
    char *ptr = strtok(copyLine, delim);

    while(ptr != NULL)
    {
        words = (char**) realloc(words, (*nrWords + 1) * sizeof(char*));
        words[*nrWords] = malloc(25 * sizeof(char));
        strcpy(words[(*nrWords)++], ptr);
        // printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);
    }

    free(ptr);
    return words;
}

void freeWords(char **words, int nrWords) {
    for (int i = 0; i < nrWords; ++i)
        free(words[i]);

    free(words);
}

void freeNode(TreeNode *node) {
    for (int i = 0; i < node->nrChilds; ++i) {
        freeNode(node);
        free(node->childs[i]);
    }
}

TreeNode* findPath(TreeNode* node, char path[], int limit) {
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

    for (int j = 0; j < nrWords - limit && node != NULL; j++) {
        int ok = 0;
        if (strcmp(words[j], "..") == 0) {
            node = node->father;
            if (node == NULL) {
                freeWords(words, nrWords);
                return NULL;
            }
            continue;
        }

        if (strcmp(words[j], ".") == 0) {
            continue;
        }

        for (int i = 0; i < node->nrChilds; ++i)
            if (strcmp(node->childs[i]->name, words[j]) == 0) {
                ok = 1;
                node = node->childs[i];
                break;
            }

        if (!ok) {
            freeWords(words, nrWords);
            return NULL;
        }
    }

    freeWords(words, nrWords);
    return node;
}

void addPath(TreeNode *node, char dir[PATH_LEN], FileType type) {
    if (findPath(node, dir, 0) != NULL) {
        fprintf(err, "%s: already exists\n", dir);
        return;
    }

    node = findPath(node, dir, 1);
    if (node == NULL) {
        fprintf(err, "%s: No such file or directory\n", dir);
        return;
    }

    int nrWords = 0;
    char delim[] = "/";
    char** words = splitLine(dir, delim, &nrWords);
    char path[256];
    strcpy(path, words[nrWords - 1]);
    freeWords(words, nrWords);

    TreeNode **newChilds = (TreeNode**) realloc(node->childs, (node->nrChilds + 1) * sizeof(TreeNode*));
    if (newChilds != NULL) {
        // free(node->childs);
        node->childs = newChilds;
        node->childs[node->nrChilds] = createTreeNode(node);
        node->childs[node->nrChilds]->type = type;
        strcpy(node->childs[node->nrChilds++]->name, path);
    }

    // free(newChilds);
}

int copyPath(TreeNode *root, char path[], char destPath[]) {
    TreeNode *src = findPath(root, path, 0);
    if (src == NULL) {
        fprintf(err, "%s: No such file or directory\n", path);
        return 0;
    }

    TreeNode *dest = findPath(root, destPath, 0);
    if (dest == NULL) {
        fprintf(err, "%s: No such file or directory\n", destPath);
        return 0;
    }

    if (dest->type != Folder) {
        fprintf(err, "%s: Not a directory\n", destPath);
        return 0;
    }

    addPath(dest, src->name, src->type);
    return 1;
}

void removePath(TreeNode *node, char dir[PATH_LEN], FileType type) {
    node = findPath(node, dir, 0);
    if (node == NULL) {
        fprintf(err, "%s: No such file or directory\n", dir);
        return;
    }

    char filename[26];
    strcpy(filename, node->name);
    node = node->father;
    if (node == NULL)
        node = root;

    for (int i = 0; i < node->nrChilds; ++i)
        if (strcmp(node->childs[i]->name, filename) == 0) {
            if (type == Folder) {
                if (node->childs[i]->nrChilds > 0) {
                    fprintf(err, "%s: Directory not empty\n", dir);
                    return;
                }
                if (node->childs[i]->type == File) {
                    fprintf(err, "%s: Not a directory\n", dir);
                    return;
                }
            }

            // node->childs[i] = NULL; // TODO: fix memory leak
            // for (int j = i; j < node->nrChilds - 1; ++j)
                // node->childs[j] = node->childs[j+1];
            TreeNode *aux = node->childs[i];
            node->childs[i] = node->childs[node->nrChilds - 1];

            free(aux);
            node->nrChilds--;
            return;
        }
}

void movePath(TreeNode *root, char path[], char destPath[]) {
    if (!copyPath(root, path, destPath)) {
        return;
    }

    TreeNode *src = findPath(root, path, 0);
    if (src == NULL) {
        fprintf(err, "%s: No such file or directory\n", path);
        return;
    }

    removePath(src->father, src->name, File);
}

void pwdHelper(TreeNode *currDir, TreeNode *b) {
    if (currDir->father != NULL)
            pwdHelper(currDir->father, b);

    fprintf(out, "%s", currDir->name);
    if (currDir->type == Folder && currDir != root &&
        currDir != b)
        fprintf(out, "/");
}

void pwd(TreeNode* currDir) {
    pwdHelper(currDir, currDir);
}

int ls(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords > 3) {
        return 0;
    } else {
        int explicitFormat = 0;
        if (nrWords >= 2) {
            if (strcmp(words[1], "-F") == 0)
                explicitFormat = 1;
            else
                currDir = findPath(currDir, words[1], 0);
        }

        if (nrWords == 3) {
            currDir = findPath(currDir, words[2], 0);
            if (currDir == NULL) {
                fprintf(err, "%s: No such file or directory\n", words[2]);
                return 1;
            }
        }

        for (int i = 0; i < currDir->nrChilds - 1; ++i)
            for (int j = i + 1; j < currDir->nrChilds; ++j)
                if (strcmp(currDir->childs[i]->name, currDir->childs[j]->name) > 0) 
                {
                    TreeNode *aux = currDir->childs[j];
                    currDir->childs[j] = currDir->childs[i];
                    currDir->childs[i] = aux;
                }

        for (int i = 0; i < currDir->nrChilds; ++i)
            if (explicitFormat) {
                if (currDir->childs[i]->type == Folder)
                    fprintf(out, "%s/ ", currDir->childs[i]->name);
                else
                    fprintf(out, "%s* ", currDir->childs[i]->name);
            } else {
                fprintf(out, "%s ", currDir->childs[i]->name);
            }
        fprintf(out, "\n");

        return 1;
    }
}

TreeNode* generateTree() {
    if (root == NULL) {
        root = createTreeNode(NULL);
        root->type = Folder;
        strcpy(root->name, "/");
    }

    addPath(root, "ana", Folder);
    addPath(root, "abc", Folder);
    addPath(root, "manual", Folder);

    TreeNode* node = findPath(root, "abc", 0);
    addPath(node, "a", Folder);
    addPath(node, "b", Folder);
    addPath(node, "c", Folder);

    node = findPath(root, "manual", 0);
    addPath(node, "page1", File);
    addPath(node, "page2", File);
    addPath(node, "page3", File);

    node = findPath(root, "ana", 0);
    addPath(node, "are", Folder);

    node = findPath(node, "are", 0);
    addPath(node, "mere", Folder);

    return root;
}

char* readLine(FILE *in) {
    size_t bufsize = 64;
    char* buffer = malloc(bufsize * sizeof(char));

    fgets(buffer, bufsize, in);
    /* remove newline, if present */
    int i = strlen(buffer) - 1;
    if( buffer[i] == '\n')
        buffer[i] = '\0';

    return buffer;
}

TreeNode* cd(TreeNode* currDir, char** words, int nrWords) {
    if (nrWords < 2) {
        fprintf(err, "cd: missing operand\n");
    } else if (nrWords > 2) {
        fprintf(err, "cd: too many arguments\n");
    } else {
        char path[256];
        strcpy(path, words[1]);
        TreeNode* node = findPath(currDir, words[1], 0);
        if (node != NULL) {
            if (node->type == File) {
                fprintf(err, "%s: Not a directory\n", words[1]);
            } else {
                currDir = node;
            }
        } else {
            fprintf(err, "%s: No such file or directory\n", path);
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
        for (int i = 1; i < nrWords - 1; ++i) {
            TreeNode* s = findPath(currDir, words[i], 0);
            if (s == NULL || s->type == Folder) {
                fprintf(err, "%s: No such file or directory\n", words[i]);
                continue;
            }

            TreeNode* d = findPath(currDir, destPath, 0);
            if (d == NULL || d->type == Folder) {
                fprintf(err, "%s: No such file or directory\n", destPath);
                return 1;
            }
            movePath(currDir, words[i], destPath);
        }

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

TreeNode* executeCommand(TreeNode* currDir, char** words, int nrWords, char *line) {
    if (nrWords == 0)
        return currDir;

    if (strcmp(words[0], "pwd") == 0) {
        pwd(currDir);
        fprintf(out, "\n");
    }
    else if (strcmp(words[0], "ls") == 0) {
        if(!ls(currDir, words, nrWords))
            fprintf(err, "%s: too many arguments\n", line);
    } else if (strcmp(words[0], "cd") == 0) {
        TreeNode* node = cd(currDir, words, nrWords);
        currDir = node;
    } else if (strcmp(words[0], "mkdir") == 0) {
        if (!createFiles(currDir, words, nrWords, Folder))
            fprintf(err, "mkdir: missing operand\n");
    } else if (strcmp(words[0], "touch") == 0) {
        if (!createFiles(currDir, words, nrWords, File))
            fprintf(err, "touch: missing operand\n");
    }  else if (strcmp(words[0], "rmdir") == 0) {
        if (!deleteFiles(currDir, words, nrWords, Folder))
            fprintf(err, "rmdir: missing operand\n");
    }  else if (strcmp(words[0], "cp") == 0) {
        if (!copyFiles(currDir, words, nrWords))
            fprintf(err, "cp %s: missing operand\n", words[1]);
    }  else if (strcmp(words[0], "mv") == 0) {
        if (!moveFiles(currDir, words, nrWords))
            fprintf(err, "mv %s: missing operand\n", words[1]);
    }  else if (strcmp(words[0], "rm") == 0) {
        if (!deleteFiles(currDir, words, nrWords, File))
            fprintf(err, "rm: missing operand\n");
    } else if (nrWords > 0) {
        fprintf(err, "%s: command not found\n", words[0]);
    }

    return currDir;
}

void run() {
    root = generateTree();
    TreeNode *currDir = root;
    in = stdin;
    out = stdout;
    err = stdout;
    char *line;
    char delim[] = " ";
    char** words = NULL;
    int nrWords = 0;

    while(1) {
        line = readLine(in);
        words = splitLine(line, delim, &nrWords);
        currDir = executeCommand(currDir, words, nrWords, line);

        freeWords(words, nrWords);
        free(line);
        nrWords = 0;
    }

    freeNode(root);
}

void runBg() {
    in = fopen("input.in", "r");
    if (in == NULL) {
        fprintf(stderr, "ERROR: Can't open file %s\n", "input.in");
        return;
    }

    char outputFileName[] = "output.out";
    char errorFileName[] = "error.out";
    out = fopen(outputFileName, "w");
    err = fopen(errorFileName, "w");

    root = createTreeNode(NULL);
    root->type = Folder;
    strcpy(root->name, "/");

    TreeNode *currDir = root;
    char *line;
    char delim[] = " ";
    char** words = NULL;
    int nrWords = 0;
    int nrCommands = 0;

    fscanf(in, "%d", &nrCommands);
    free(readLine(in));
    while(nrCommands--) {
        line = readLine(in);
        words = splitLine(line, delim, &nrWords);
        currDir = executeCommand(currDir, words, nrWords, line);

        freeWords(words, nrWords);
        free(line);
        nrWords = 0;
    }

    free(root);
    fclose(in);
    fclose(out);
    fclose(err);
}

int main()
{
    // run();
    runBg();
}
