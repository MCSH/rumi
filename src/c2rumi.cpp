#include <clang-c/Index.h>
#include <iostream>
#include <string>
using namespace std;

ostream &operator<<(ostream &stream, const CXString &str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

string translate_type(CXType t){
  auto str = clang_getTypeSpelling(t);
  string type = clang_getCString(str);

  if(t.kind == CXType_Pointer){
    return "*" + translate_type(clang_getPointeeType(t));
  }

  if(t.kind == CXType_Elaborated){
    // struct....
    auto c = clang_getTypeDeclaration(t);
    return clang_getCString(clang_getCursorSpelling(c));
  }
  
  if(type == "void"){
    return "unit";
  }
  return type;
}

struct MyData{
  bool is_inside_brackets;
};

void functionVisitor(CXCursor c){

  auto type = clang_getCursorType(c);

  auto resType = clang_getResultType(type);

  auto name = clang_getCursorSpelling(c);

  if(type.kind == CXType_FunctionNoProto){
    cout << name << " := () -> " << translate_type(resType) << ";" << endl;
  } else {
    cout << name << " := (";
    int total = clang_getNumArgTypes(type);

    for(int i = 0; i < total; i++){
      auto argType = clang_getArgType(type, i);
      cout << "v" << i << ": "<< translate_type(argType);

      if(i != total - 1){
        cout << ", ";
      }

    }

    cout << ") -> " << translate_type(resType) << ";" << endl;
  }
}

CXChildVisitResult visitor(CXCursor c, CXCursor parent, CXClientData clientData){

  MyData *data = (MyData*)clientData;

  CXCursorKind kind = clang_getCursorKind(c);

  // handle the open brackets
  if (data->is_inside_brackets) {
    switch (kind) {
    case CXCursor_FunctionDecl:
    case CXCursor_StructDecl:
      cout << "};" << endl;
      data->is_inside_brackets = false;
      break;
    }
  }

  switch (kind) {
  case CXCursor_StructDecl:
    // cout << "StructDecl: " << clang_getCursorSpelling(c) << endl;
    cout << clang_getCursorSpelling(c) << " : struct {" << endl;
    data->is_inside_brackets = true;
    return CXChildVisit_Recurse;
    break;

  case CXCursor_FieldDecl:
    cout << "  " << clang_getCursorSpelling(c) << " : " << translate_type(clang_getCursorType(c)) << ";" << endl;
    return CXChildVisit_Continue;

  case CXCursor_FunctionDecl:
    functionVisitor(c);
    return CXChildVisit_Continue;

  default:
    cout << "// Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
         << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "'\n";
    return CXChildVisit_Continue;
  }
  }

void process(char *filename) {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, filename, nullptr, 0, nullptr, 0, CXTranslationUnit_None);
  if (unit == nullptr) {
    cerr << "Unable to parse translation unit. Quitting." << endl;
    exit(-1);
  }

  CXCursor cursor = clang_getTranslationUnitCursor(unit);

  MyData data;

  data.is_inside_brackets = false;
  
  clang_visitChildren(cursor, visitor, &data);

  if(data.is_inside_brackets){
    cout << "};" << endl;
  }

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

int main(int argc, char **argv){
  for(int i = 1; i < argc; i++){
    process(argv[i]);
  }

  return 0;
}
