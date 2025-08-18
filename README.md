# PDF Presenter

![PDF Presenter logo](resources/assets/logo_full.png)

This tool purpose is to replicate the *Presenter Mode* of MS PowerPoint for PDF files, and especially for LaTeX presentation.

## About this tool

The *Presentor Mode* of MS PowerPoint is great, but MS PowerPoint sucks! As presentation made with [LaTeX](https://www.latex-project.org/) are more professional, the only way to present them was using a PDF viewer and put it in full-screen. With two displays, them would output the same thing: the current page the presentation file. We assume that the presenter has the display of its own computer, and uses an external display to actually displays the presentation. We wanted to take advantage of the presenter's display, by displaying useful pieces of information like in MS PowerPoint. These are, but are not limited to:
- The current slide;
- The next slide;
- The local time;
- A chronometer;
- Notes for each slide.

Although, there is already another tool available called [Showcase PDF](https://github.com/russkyc/showcase-pdf). We made abstraction of the availability of this piece of software, since it must convert each slide (each PDF page) to an image, and is written in C# needing [Microsoft's SDK](https://dotnet.microsoft.com/en-us/download) to be installed on the running computer.

The goal with this project is to offer a simple, fast, powerful and native tool to present your PDF files.

## How to use it?

PDF Presenter does accept command-line arguments. The first argument is a relative/absolute path to the PDF path to open. The second argument is a relation/absolute path to the notes file to open. The notes file must be a MarkDown or a text file.

In order to display appropriate notes on each slide in the notes section, PDF Presenter parses the notes file and looks for a delimiter. In a MarkDown file, the delimiter is `\n# `. In a text file, the delimiter is `\n---\n`. We plan to implement a feature letting the user choose a custom delimiter for any file.

To get help, use the `-h` or `--help` as first command line argument.

``` text
$ pdf-presenter -h
PDF Presenter
Maintainer: Andrei Zeucianu
GitHub repository: https://github.com/Androl404/pdf-presenter

Usage:
    pdf-presenter [pdf_file.pdf] [notes_file.{txt,md}]

Help:
    -h, --help       Show this help message
    -v, --version    Show this software's version
```

PDF presenter also has a monitors list allowing you to choose on which window to put the presentation window. The presentation window is automatically put in full-screen mode, but you can revert that with your window manager and move the window wherever you want.

## Getting this tool
### Building it from source code
#### Building on Unix like operating systems

The main dependencies of this project are `gtk4`, `poppler` and `cairo`. The development files of these libraries must be installed on your system. For instance, to install these packages on Debian, you would run:

``` shell
# apt install libgtk-4-dev libcairo2-dev libpoppler-dev libpoppler-glib-dev
```

On different Unix like operating systems, please refer to your operating system documentation and to your package manager.

After that, you only need to run a few commands in order to compile and run PDF Presenter:

```shell
$ git clone https://github.com/Androl404/pdf-presenter
$ cd pdf-presenter
$ ./build.sh
$ ./build/pdf-presenter
```

In order to clean the project, simply run:
```shell
$ ./build.sh clean
```

This will delete the `build` folder at the root of the repository, containing only the final executable of PDF Presenter.

> [!NOTE]
> This project does not natively support MacOS, but we welcome changes to add support for new targets!

> [!TIP]
> FreeBSD is supported and the `build.sh` file also runs on that operating system.

#### Building on MS Windows

Building on MS Windows is a little bit trickier, but is supported this project. We recommend installing [msys2](https://www.msys2.org/) which is a collection of tools and libraries with an easy-to-use environment for building, installing and running native Windows software. After installing that dependency, open the start menu and search for MSYS2 MINGW64. Run the following commands to update msys2 and to install all the required dependencies:

```sh
$ pacman -Suy
$ pacman -S mingw-w64-x86_64-gtk4 mingw-w64-x86_64-gcc pkgconf mingw-w64-x86_64-poppler mingw-w64-x86_64-glib2
```

After the installation has succeeded, you must add the following folder `C:\msys64\mingw64\bin` to the Windows path as first element. Restart your computer, reopen the same console and go to your C drive (`cd /c/`) in the folder you want to clone this project. Then, follow the *Unix like operating system* instructions to build this project.

### Pre-built binaries

Pre-built binaries are available in the [release page](https://github.com/Androl404/pdf-presenter/releases). Pre-built binaries for Windows are available as a `zip` archive with PDF Presenter and GTK's binaries.

A pre-built binary is also available for [Ubuntu 24.04](https://ubuntu.com/download/desktop?version=24.04&architecture=amd64&lts=true). This binaries should work on any recent Linux distribution (tested on Debian 13). This binary consists on a single executable file.

#### Binaries on Windows

In order to build and distribute the application from MS Windows, first follow the building instructions for MS Windows. Then, install the Adwaita dependency with:

``` shell
$ pacman -S mingw-w64-x86_64-libadwaita
```

Make sure package `mingw-w64-x86_64-gtk4` is installed. Build your project with a custom prefix (e.g. ~/my-gtk-app-prefix). Navigate to this directory such that you have subdirectories 'bin', 'lib', 'share', 'etc'. The executable should be distributed in the 'bin' folder.

The following command copies all dependent DLLs to the current directory:

``` shell
$ ldd ./bin/pdf-presenter.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" ./bin
```

This ensures that the necessary DLLs are bundled in the application folder so the end user does not need their own installation of msys2. GTK will crash if it cannot load images. GdkPixbuf needs various loaders for different image formats. Dependent DDLs must also be copied.

``` shell
$ cp -r /mingw64/lib/gdk-pixbuf-2.0 ./lib/gdk-pixbuf-2.0
$ ldd lib/gdk-pixbuf-2.0/2.10.0/loaders/pixbufloader_svg.dll | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" ./bin
```

We then need to copy over the Adwaita and hicolor icon themes, otherwise GTK will display a fallback 'missing icon' symbol.

``` shell
$ cp -r /mingw64/share/icons/* ./share/icons/
```

The relevant settings schemas need to be installed. Copy them over and recompile. You may want to manually inspect the copied over schemas (in case unrelated projects are also copied).

``` shell
$ cp /mingw64/share/glib-2.0/schemas/* ./share/glib-2.0/schemas/
$ glib-compile-schemas.exe ./share/glib-2.0/schemas/
```

References:
- <https://vidhukant.com/blog/distributing-gtk-app-for-windows/>
- <https://gist.github.com/albertgoss/a0f38e83d6634e31527c33bb474ae1a7>

#### Binaries icon

On GNU/Linux, the FreeDesktop conventions impose the use of a `.desktop` file in order to set the icon if the application in desktop environment.

On Windows, there are multiple ways to set the executable icon with GTK and Meson. Here are multiple references:
- <https://discourse.gnome.org/t/gtk4-icons-on-windows/17129/5>
- <https://stackoverflow.com/questions/64545983/how-do-i-add-an-icon-to-an-iconless-exe-file>

We decided to use [RessourceHacker](https://www.angusj.com/resourcehacker/) to set the icon of our application following the second previous reference. The ICO file available in `resources/assets/logo_pdf_square/logo_pdf_square_256.ico` is used.

## Screenshots

![Screenshot of PDF Presenter](resources/screenshots/Screenshot From 2025-08-18 16-45-38.png)
![Screenshot of PDF Presenter](resources/screenshots/Screenshot From 2025-08-18 16-48-50.png)
![Screenshot of PDF Presenter](resources/screenshots/Screenshot From 2025-08-18 16-50-58.png)
![Screenshot of PDF Presenter](resources/screenshots/Screenshot From 2025-08-18 16-51-16.png)
![Screenshot of PDF Presenter](resources/screenshots/Screenshot From 2025-08-18 16-51-34.png)

## Known bugs

- Specifically on the [i3 tiling window manager](https://i3wm.org/), the displays function is not functioning properly, since the presentation popup will go full-screen on the monitor containing the control window, and not on the specified monitors in the "Displays" section.
- Spawning multiple instances of this application results in a broken behaviour.

## Contributing

Any contribution is welcome. Please file a Pull Request describing the changes you want to merge. This project tries to follow the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) convention, although we are quite flexible with the commits message, while it does its best to describe the change which is made.

This project also uses [semantic versioning](https://semver.org/).

## Licence

See the [LICENCE](./LICENSE) file. In short, this project is licensed under the MIT license.
