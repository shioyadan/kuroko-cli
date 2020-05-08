%comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

https://franklinheath.co.uk/2015/08/29/custom-page-sizes-for-microsoft-print-to-pdf/


## How to build

* Install Visual Studio Build Tools 2019 - C++ Build Tools 
* Run this command
    ```
    build_shell.bat nmake build_release 
    ```
* or  
    ```
    C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat
    nmake build_release
    ```