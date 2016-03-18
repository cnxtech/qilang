/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <qi/log.hpp>
#include <qilang/node.hpp>
#include <qilang/formatter.hpp>
#include <qi/os.hpp>
#include "formatter_p.hpp"
#include "cpptype.hpp"
#include <qilang/visitor.hpp>

qiLogCategory("qigen.cppremote");

namespace qilang {

  class CppAsyncRemoteQiLangGen: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    CppAsyncRemoteQiLangGen(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "class " << node->name + "AsyncRemote" << ": public " << node->name << "Async";
      //there is some inherits, so proxy is already inherited by the parent.
      if (node->inherits.size() > 0) {
        for (unsigned i = 0; i < node->inherits.size(); ++i) {
          out() << ", public " << node->inherits.at(i) << "AsyncRemote";
        }
      }
      out() << " {" << std::endl;

      indent() << "public:" << std::endl;
      {
        ScopedIndent _(_indent);

        indent() << node->name + "AsyncRemote(const qi::AnyObject& ao)" << std::endl;
        {
          ScopedIndent _(_indent);
          indent() << ": ";
          for (unsigned i = 0; i < node->inherits.size(); ++i) {
            out() << node->inherits[i] << "AsyncRemote(ao)" << std::endl;
            indent() << ", ";
          }
        }
        out() << "_obj(ao)" << std::endl;
        indent() << "{}" << std::endl;

        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }
      }

      out() << std::endl;
      indent() << "private:" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "qi::AnyObject _obj;" << std::endl;
      }

      indent() << "};" << std::endl;
      out() << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      indent() << "qi::Future< ";
      accept(node->effectiveRet());
      out() << " > " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ") {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "return _obj.async< ";
        accept(node->effectiveRet());
        out() << " >(";
        out() << "\"" << node->name << "\"";
        if (node->args.size() != 0)
          out() << ", ";
        cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
        out() << ");" << std::endl;
      }
      indent() << "}" << std::endl;
    }
    void visitDecl(SigDeclNode* node) {
    }
    void visitDecl(PropDeclNode* node) {
    }
  };

  class CppProxySigPropQiLangGen: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    CppProxySigPropQiLangGen(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    void visitDecl(FnDeclNode* node) {}
    void visitDecl(SigDeclNode* node) {
      indent() << "qi::makeProxySignal(_" << node->name << ", ao, \"" << node->name << "\");" << std::endl;
    }
    void visitDecl(PropDeclNode* node) {
      indent() << "qi::makeProxyProperty(_" << node->name << ", ao, \"" << node->name << "\");" << std::endl;
    }
  };

  class CppDeclareSigPropQiLangGen: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    CppDeclareSigPropQiLangGen(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    void visitDecl(FnDeclNode* node) {}
    void visitDecl(SigDeclNode* node) {
      indent() << "qi::Signal< ";
      ScopedFormatAttrBlock _(constattr);
      cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
      out() << " > _" << node->name << ";" << std::endl;
    }
    void visitDecl(PropDeclNode* node) {
      indent() << "qi::Property< ";
      ScopedFormatAttrBlock _(constattr);
      cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
      out() << " > _" << node->name << ";" << std::endl;
    }
  };

  class CppSyncRemoteQiLangGen: public CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >
  {
  public:
    CppSyncRemoteQiLangGen(std::stringstream& ss, int indent)
      : CppTypeFormatter<NodeFormatter<DefaultNodeVisitor> >(ss, indent)
    {}

    void visitDecl(InterfaceDeclNode* node) {
      indent() << "class " << node->name + "Remote" << ": public " << node->name;
      //there is some inherits, so proxy is already inherited by the parent.
      if (node->inherits.size() == 0) {
        out() << ", public qi::Proxy {" << std::endl;
      } else {
        for (unsigned i = 0; i < node->inherits.size(); ++i) {
          out() << ", public " << node->inherits.at(i) << "Remote";
        }
        out() << " {" << std::endl;
      }

      indent() << "public:" << std::endl;

      {
        ScopedIndent _(_indent);

        indent() << node->name + "Remote(const qi::AnyObject& ao)" << std::endl;
        {
          ScopedIndent _(_indent);
          indent() << ": ";
          for (unsigned i = 0; i < node->inherits.size(); ++i) {
            out() << node->inherits[i] << "Remote(ao)" << std::endl;
            indent() << ", ";
          }

          out() << node->name << "(" << std::endl;
          {
            bool first = true;
            for (unsigned int i = 0; i < node->values.size(); ++i) {
              if (node->values.at(i)->type() == NodeType_SigDecl) {
                if (first)
                  first = false;
                else
                  out() << ", ";
                out() << "_" << boost::static_pointer_cast<SigDeclNode>(node->values.at(i))->name;
              }
              if (node->values.at(i)->type() == NodeType_PropDecl) {
                if (first)
                  first = false;
                else
                  out() << ", ";
                out() << "_" << boost::static_pointer_cast<PropDeclNode>(node->values.at(i))->name;
              }
            }
          }
          out() << ")" << std::endl;
          indent() << ", qi::Proxy(ao)" << std::endl;
          indent() << ", _async(ao)" << std::endl;
        }
        indent() << "{" << std::endl;
        {
          ScopedIndent _(_indent);

          CppProxySigPropQiLangGen proxy(out(), _indent);
          node->accept(&proxy);
        }
        indent() << "}" << std::endl;

        for (unsigned int i = 0; i < node->values.size(); ++i) {
          accept(node->values.at(i));
        }

        indent() << node->name << "Async& async() {" << std::endl;
        {
          ScopedIndent _(_indent);
          indent() << "return _async;" << std::endl;
        }
        indent() << "}" << std::endl;
      }

      out() << std::endl;
      indent() << "private:" << std::endl;
      {
        ScopedIndent _(_indent);

        CppDeclareSigPropQiLangGen decl(out(), _indent);
        node->accept(&decl);

        indent() << node->name << "AsyncRemote _async;" << std::endl;
      }

      indent() << "};" << std::endl;
      indent() << "QI_REGISTER_PROXY_INTERFACE(" << node->name + "Remote, " << node->name << ");" << std::endl;
      indent() << std::endl;
    }

    void visitDecl(FnDeclNode* node) {
      indent();
      accept(node->effectiveRet());
      out() << " " << node->name << "(";
      cppParamsFormat(this, node->args);
      out() << ") {" << std::endl;
      {
        ScopedIndent _(_indent);
        if (!node->hasNoReturn())
        {
          indent() << "return _obj.call< ";
          accept(node->ret);
          out() << " >(";
        }
        else
          indent() << "_obj.call<void>(";
        out() << "\"" << node->name << "\"";
        if (node->args.size() != 0)
          out() << ", ";
        cppParamsFormat(this, node->args, CppParamsFormat_NameOnly);
        out() << ");" << std::endl;
      }
      indent() << "}" << std::endl;
    }

    void visitDecl(SigDeclNode* node) {
      indent() << "qi::Signal< ";
      ScopedFormatAttrBlock _(constattr);
      cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
      out() << " >& " << node->name << "() {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "return _" << node->name << ";" << std::endl;
      }
      indent() << "}" << std::endl;
    }
    void visitDecl(PropDeclNode* node) {
      indent() << "qi::Property< ";
      ScopedFormatAttrBlock _(constattr);
      cppParamsFormat(this, node->args, CppParamsFormat_TypeOnly);
      out() << " >& " << node->name << "() {" << std::endl;
      {
        ScopedIndent _(_indent);
        indent() << "return _" << node->name << ";" << std::endl;
      }
      indent() << "}" << std::endl;
    }
  };

  //Generate Type Registration Information
  class CppRemoteQiLangGen: public CppTypeFormatter<>
  {
  public:
    CppRemoteQiLangGen(const PackageManagerPtr& pm, const StringVector& includes)
      : _includes(includes)
    {}

    void doAccept(Node* node) { node->accept(this); }

    StringVector currentNs;
    StringVector _includes;

    void visitDecl(InterfaceDeclNode* node) {
      CppAsyncRemoteQiLangGen ar(out(), _indent);
      node->accept(&ar);
      CppSyncRemoteQiLangGen ir(out(), _indent);
      node->accept(&ir);
    }

    void visitDecl(ParamFieldDeclNode* node) {
    }

    void visitDecl(FnDeclNode* node) {
    }

    void visitDecl(SigDeclNode* node) {
    }
    void visitDecl(PropDeclNode* node) {
    }

    void walkParams(const ParamFieldDeclNodePtrVector& params) {
      for (unsigned i = 0; i < params.size(); ++i) {
        accept(params.at(i));
        if (i + 1 < params.size())
          out() << ", ";
      }
    }

    void visitDecl(StructDeclNode* node) {
      qiLogError() << "StructDeclNode not implemented";
    }
    void visitDecl(ConstDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
    void visitDecl(StructFieldDeclNode* node) {
      qiLogError() << "FieldDeclNode not implemented";
    }
    void visitDecl(EnumDeclNode* node) {
      qiLogError() << "EnumDeclNode not implemented";
    }
    void visitDecl(EnumFieldDeclNode* node) {
      qiLogError() << "EnumFieldDeclNode not implemented";
    }
    void visitDecl(TypeDefDeclNode* node) {
      qiLogError() << "TypeDefDeclNode not implemented";
    }

  void formatHeader() {
    indent() << "/*" << std::endl;
    indent() << "** qiLang generated file. DO NOT EDIT" << std::endl;
    indent() << "*/" << std::endl;
    indent() << "#include <qi/type/objecttypebuilder.hpp>" << std::endl;
    for (unsigned i = 0; i < _includes.size(); ++i) {
      indent() << "#include " << _includes.at(i) << std::endl;
    }
    indent() << std::endl;
  }

  void formatFooter() {
    for (int i = 0; i < currentNs.size(); ++i) {
      out() << "}" << std::endl;
    }
  }

  void visitStmt(PackageNode* node) {
    currentNs = splitPkgName(node->name);
    for (unsigned int i = 0; i < currentNs.size(); ++i) {
      indent() << "namespace " << currentNs.at(i) << " {" << std::endl;
    }
    out() << std::endl;
  }

  void visitStmt(ImportNode* node) {
  }

  void visitStmt(ObjectDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(PropertyDefNode *node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(AtNode* node) {
    throw std::runtime_error("unimplemented");
  }
  void visitStmt(VarDefNode* node) {
    throw std::runtime_error("unimplemented");
  }

};

std::string genCppObjectRemote(const PackageManagerPtr& pm, const ParseResultPtr& pr) {
  StringVector sv = extractCppIncludeDir(pm, pr, true);
  return CppRemoteQiLangGen(pm, sv).format(pr->ast);
}



}

