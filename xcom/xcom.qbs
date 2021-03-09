import qbs

StaticLibrary {
    Depends
    {
        name: "cpp"
    }
    files: [
        "inc/stdext/secured.hpp",
        "api/xcom.hpp",
        "inc/xcom/client.hpp",
        "inc/xcom/common.hpp",
        "inc/xcom/server.hpp",
        "src/xcom/client.cpp",
        "src/xcom/common.cpp",
        "src/xcom/server.cpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.enableRtti: false
    cpp.includePaths: ["api", "inc"]
    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "lib"
    }
}
