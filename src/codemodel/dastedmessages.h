#ifndef DASTEDMESSAGES_H
#define DASTEDMESSAGES_H

#include <vector>
#include <string>
#include <limits>

#include <msgpack.hpp>

namespace Dasted {

const quint8 PROTOCOL_VERSION = 4;

struct DString
{
    std::string impl;
    void msgpack_unpack(const msgpack::object &o)
    {
        impl.clear();
        if (o.type != msgpack::type::NIL) {
            if (o.type != msgpack::type::STR) {
                throw std::runtime_error("bad msgpack type (string is expected)");
            }
            o >> impl;
        }
    }

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack(impl);
    }
};

template <class T>
struct DVector
{
    std::vector<T> impl;
    void msgpack_unpack(const msgpack::object &o)
    {
        impl.clear();
        if (o.type != msgpack::type::NIL) {
            if (o.type != msgpack::type::ARRAY) {
                throw std::runtime_error("bad msgpack type (array is expected)");
            }
            o >> impl;
        }
    }

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack(impl);
    }
};

struct Location
{
    DString filename;
    uint cursor;

    MSGPACK_DEFINE(filename, cursor)
};

struct Sources
{
    DString filename;
    uint revision;
    DString text;

    MSGPACK_DEFINE(filename, revision, text)
};

typedef unsigned char SymbolType;
const uint NO_REVISION = 0;

struct Symbol
{
    SymbolType type;
    Location location;
    DString name;
    DString typeName;
    DVector<DString> qualifiers;
    DVector<DString> parameters;
    DVector<DString> templateParameters;
    DString doc;

    MSGPACK_DEFINE(type, location, name, typeName, qualifiers, parameters, templateParameters, doc)
};

struct Scope
{
    Symbol symbol;
    DVector<Scope> children;

    MSGPACK_DEFINE(symbol, children)
};

enum MessageType
{
    WRONG_TYPE = 0,
    COMPLETE = 1,
    FIND_DECLARATION = 2,
    ADD_IMPORT_PATHS = 3,
    GET_DOC = 4,
    OUTLINE = 5,
};

template <MessageType T> struct Request;

template <>
struct Request<COMPLETE>
{
    enum {type = COMPLETE};
    DString project;
    Sources src;
    uint cursor;

    MSGPACK_DEFINE(project, src, cursor)
};

template <>
struct Request<FIND_DECLARATION>
{
    enum {type = FIND_DECLARATION};
    DString project;
    Sources src;
    uint cursor;

    MSGPACK_DEFINE(project, src, cursor)
};

template <>
struct Request<ADD_IMPORT_PATHS>
{
    enum {type = ADD_IMPORT_PATHS};
    DString project;
    DVector<DString> paths;

    MSGPACK_DEFINE(project, paths)
};

template <>
struct Request<GET_DOC>
{
    enum {type = GET_DOC};
    DString project;
    Sources src;
    uint cursor;

    MSGPACK_DEFINE(project, src, cursor)
};

template <>
struct Request<OUTLINE>
{
    enum {type = OUTLINE};
    DString project;
    Sources src;

    MSGPACK_DEFINE(project, src)
};

template <MessageType T> struct Reply;

template <>
struct Reply<COMPLETE>
{
    enum {type = COMPLETE};
    bool calltips;
    DVector<Symbol> symbols;

    MSGPACK_DEFINE(calltips, symbols)
};

template <>
struct Reply<FIND_DECLARATION>
{
    enum {type = FIND_DECLARATION};
    Symbol symbol;

    MSGPACK_DEFINE(symbol)
};


template <>
struct Reply<ADD_IMPORT_PATHS>
{
    enum {type = ADD_IMPORT_PATHS};
    unsigned char payload;

    MSGPACK_DEFINE(payload)
};

template <>
struct Reply<GET_DOC>
{
    enum {type = GET_DOC};
    DVector<Symbol> symbols;

    MSGPACK_DEFINE(symbols)
};

template <>
struct Reply<OUTLINE>
{
    enum {type = OUTLINE};
    Scope global;

    MSGPACK_DEFINE(global)
};

} // namespace Dasted

#endif // DASTEDMESSAGES_H
