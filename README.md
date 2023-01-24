

# Vau

**Vau** is an experiment/exercise over the TeXmacs codebase, to learn more about it. The initial goal is to extract enough machinery to be able to read and typeset arbitrary TeXmacs files. So **Vau** will be initially a viewer. This will allow me to understand the code dependencies and extract a minimal typesetting core, abstracted from the UI and the wider organization of the editor. In the meanwhile I plan to experiment about various refactorings.

### Glue code refactoring

I rewrote the Scheme glue code via C++ templating and metaprogramming and make it local to each submodule. Glue code should be defined locally where the relevant C++ function belongs, so that if a module is not linked in the corresponding glue code is not compiled in and will not be visible from scheme. 


## Logbook

Jan 2022 

- I have now a basic setup with an Xcode project and a CMake recipe. I switched to the S7 interpreter, added PDF Hummus and MuPDF support. Vau can read and export to PDF fairly complex documents with PDF Hummus and generate PNG images for pages with MuPDF. Typesetting is not optimized and all the document is retypeset from the beginning at every invocation of the output routines.

- I removed almost all the C++ code which is not used at this stage. And Vau load the minimal amount of Scheme code.

- Ideally we want to isolate away I/O, because e.g. in Webasm we do not have direct access to the filesystem and maybe this requires anyway async calls.

- From here I would like to understand how to isolate a well-defined interface for the editor which allow to call it from a scripting language (be it Scheme or Javascript). The editor receives commands and send back responses (e.g. bitmaps of the current page with metadata, like location of the cursors, etc..)


