# Languages_model
A model for the evolution of humanity's number of languages ​​used in projects with Peter Culicover

## COMPILATION

To compile, you need to download the "https://github.com/borkowsk/symShell2andRTM" repository and place it in the cousin directory as below (or modify the path in the CMakeLists.txt file)

```
FolderChosenByYou/
├── arch
│   └── Languages_model
└── public
    ├── symShell2andRTM        (Inside there is a verified version of "symShellLight" in the SYMSHELL folder)
    └── symShellLight          (If you want to use the latest version)
```

## USAGE

Compiled application you can start from command line with many possible options:

```
language -gray+ AUTO=5 WIDTHWIN=150 HEIGHTWIN=51 MAX=10 LOGC=10 LOGF=log2.out
```
Most useful options:

* -gray+  - displays in grayscale
* AUTO=5  - 5 repetitions of the experiment (without waiting for the user)
* WIDTHWIN=150 HEIGHTWIN=51 - window size; here small for maximum acceleration
* MAX=10  - 10 steps of each simulation (in this case)
* LOG=10  - recording every 10 steps, i.e. at the end. The state of step 0 and the final step will be saved.
* LOGF=log2.out - output filename.
* HELP - list of all possible options.

