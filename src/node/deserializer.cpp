#include <Python.h>

#include <QDataStream>

#include "node/deserializer.h"
#include "node/node.h"
#include "node/manager.h"

#include "node/2d/circle_node.h"
#include "node/2d/triangle_node.h"
#include "node/2d/text_node.h"
#include "node/2d/point2d_node.h"
#include "node/3d/cube_node.h"
#include "node/3d/sphere_node.h"
#include "node/3d/cylinder_node.h"
#include "node/3d/extrude_node.h"
#include "node/3d/point3d_node.h"
#include "node/meta/script_node.h"
#include "node/csg/union_node.h"
#include "node/csg/intersection_node.h"
#include "node/csg/difference_node.h"
#include "node/deform/attract_node.h"
#include "node/deform/repel_node.h"
#include "node/deform/scalex_node.h"
#include "node/deform/scaley_node.h"
#include "node/deform/scalez_node.h"
#include "node/transform/rotatex_node.h"
// NODE HEADERS

#include "datum/float_datum.h"
#include "datum/name_datum.h"
#include "datum/string_datum.h"
#include "datum/script_datum.h"
#include "datum/output_datum.h"
#include "datum/shape_datum.h"
#include "datum/function_datum.h"

SceneDeserializer::SceneDeserializer(QObject* parent)
    : QObject(parent)
{
    // Nothing to do here
}

void SceneDeserializer::run(QDataStream* in)
{
    QString sb;
    quint32 version_major;
    quint32 version_minor;
    *in >> sb >> version_major >> version_minor;

    deserializeNodes(in, NodeManager::manager());
    deserializeConnections(in);
}

void SceneDeserializer::deserializeNodes(QDataStream* in, QObject* p)
{
    quint32 count;
    *in >> count;
    for (unsigned i=0; i < count; ++i)
    {
        deserializeNode(in, p);
    }
}

void SceneDeserializer::deserializeNode(QDataStream* in, QObject* p)
{
    quint32 t;
    *in >> t;
    QString node_name;
    *in >> node_name;

    Node* node;
    NodeType::NodeType node_type = static_cast<NodeType::NodeType>(t);

    switch (node_type)
    {
        case NodeType::CIRCLE:
            node = new CircleNode(p); break;
        case NodeType::TRIANGLE:
            node = new TriangleNode(p); break;
        case NodeType::TEXT:
            node = new TextNode(p); break;
        case NodeType::POINT2D:
            node = new Point2D(p); break;
        case NodeType::CUBE:
            node = new CubeNode(p); break;
        case NodeType::POINT3D:
            node = new Point3D(p); break;
        case NodeType::SCRIPT:
            node = new ScriptNode(p); break;
        case NodeType::SPHERE:
            node = new SphereNode(p); break;
        case NodeType::CYLINDER:
            node = new CylinderNode(p); break;
        case NodeType::EXTRUDE:
            node = new ExtrudeNode(p); break;
        case NodeType::UNION:
            node = new UnionNode(p); break;
        case NodeType::INTERSECTION:
            node = new IntersectionNode(p); break;
        case NodeType::DIFFERENCE:
            node = new DifferenceNode(p); break;
        case NodeType::ATTRACT:
            node = new AttractNode(p); break;
        case NodeType::REPEL:
            node = new RepelNode(p); break;
        case NodeType::SCALEX:
            node = new ScaleXNode(p); break;
        case NodeType::SCALEY:
            node = new ScaleYNode(p); break;
        case NodeType::SCALEZ:
            node = new ScaleZNode(p); break;
        case NodeType::ROTATEX:
            node = new RotateXNode(p); break;
        // NODE CASES
    }
    node->setObjectName(node_name);

    // Deserialize child nodes.
    deserializeNodes(in, node);

    quint32 datum_count;
    *in >> datum_count;
    for (unsigned d=0; d < datum_count; ++d)
    {
        deserializeDatum(in, node);
    }
}

void SceneDeserializer::deserializeDatum(QDataStream* in, Node* node)
{
    quint32 t;
    *in >> t;
    QString name;
    *in >> name;

    DatumType::DatumType datum_type = static_cast<DatumType::DatumType>(t);

    Datum* datum;

    switch (datum_type)
    {
        case DatumType::FLOAT:
            datum = new FloatDatum(name, node); break;
        case DatumType::NAME:
            datum = new NameDatum(name, node); break;
        case DatumType::STRING:
            datum = new StringDatum(name, node); break;
        case DatumType::SCRIPT:
            datum = new ScriptDatum(name, node); break;
        case DatumType::SHAPE:
            datum = new ShapeDatum(name, node); break;
        case DatumType::SHAPE_OUTPUT:
            datum = new ShapeOutputDatum(name, node); break;
        case DatumType::SHAPE_FUNCTION:
            datum = new ShapeFunctionDatum(name, node); break;
    }

    EvalDatum* e = dynamic_cast<EvalDatum*>(datum);
    FunctionDatum* f = dynamic_cast<FunctionDatum*>(datum);
    if (e)
    {
        QString expr;
        *in >> expr;
        e->setExpr(expr);
    }
    else if (f)
    {
        QString function_name;
        QList<QString> function_args;
        *in >> function_name >> function_args;
        f->setFunctionName(function_name);
        f->setArguments(function_args);
    }

    datums << datum;
}
#include <QDebug>
void SceneDeserializer::deserializeConnections(QDataStream* in)
{
    quint32 count;
    *in >> count;

    for (unsigned i=0; i < count; ++i)
    {
        quint32 source_index, target_index;
        *in >> source_index >> target_index;
        datums[target_index]->addLink(datums[source_index]->linkFrom());
    }
}
