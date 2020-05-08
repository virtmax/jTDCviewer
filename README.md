## jTDCviewer

Viewer for data from jTDCs (https://github.com/jobisoft/jTDC)

## Installation

**Requirements:**
A C++ compiler with C++17 support, e.g. GNU G++ 8.0 or Microsoft Visual Studio 2019.

**On Ubuntu-like systems:**

* Execute:

    ```bash
    sudo apt install build-essential git qt5-default qtcreator g++-8
    ```

* Now you can download the code:                           

    ```bash
    git clone --recurse-submodules https://github.com/virtmax/jTDCviewer
    ```

* Start ```jTDCviewer.pro``` with QtCreator.

**On Windows systems:**

On Windows QtCreator comes with a compiler that doesn't support C++17 standard properly.
To fix that Visual Studio 2019 (its free) can be installed. After that you can use QtCreator with MSVC++ compiler or completely move to Visual Studio (with https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123).

* Install MS Visual Studio 2019 Community Edition from https://visualstudio.microsoft.com/de/downloads/
* Install QtCreator from https://www.qt.io/download (unfortunately only after a free registration). 
Install "MSVC 2019 64-bit". 

* Install Git from https://git-scm.com/downloads

* Clone this repository from GitHub:

    ```bash
        git clone --recurse-submodules https://github.com/virtmax/jTDCviewer
    ```

* Start ```jTDCviewer.pro``` with QtCreator.

## Update to a new version

To update your local files to a new version, go into the jTDCviewer directory and execute:

```bash
git pull
git submodule update --remote --merge
```

## Bugs & Features

If you find a bug, then you have following options:

* Fix the bug and commit a patch per GitHub or email 
* Create a 'New issue' on GitHub or write me an email with a very clear description of the problem
* It's not a bug, it's a feature!
