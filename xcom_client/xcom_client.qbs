import qbs

CppApplication {
    consoleApplication: true
    files: [
        "inc_ext/xcom.hpp",
        "src/main.cpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.enableRtti: false
    cpp.includePaths: ["inc", "inc_ext"]
    cpp.staticLibraries: ["pthread"]
    Depends {
        name: "xcom"
        required: true
    }
}
