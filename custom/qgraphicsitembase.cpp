#include "qgraphicsitembase.h"

QVector<QGraphicsItemBase::AuxiliaryLine> QGraphicsItemBase::auxiliaryLines;

bool QGraphicsItemBase::AuxiliaryLine::lock;
bool QGraphicsItemBase::AuxiliaryLine::hide;
