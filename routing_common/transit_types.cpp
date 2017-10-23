#include "routing_common/transit_types.hpp"

#include "routing_common/transit_serdes.hpp"

namespace
{
double constexpr kWeightEqualEpsilon = 1e-2;
double constexpr kPointsEqualEpsilon = 1e-6;
}  // namespace

namespace routing
{
namespace transit
{
// TransitHeader ----------------------------------------------------------------------------------
TransitHeader::TransitHeader(uint16_t version, uint32_t gatesOffset, uint32_t edgesOffset,
                             uint32_t transfersOffset, uint32_t linesOffset, uint32_t shapesOffset,
                             uint32_t networksOffset, uint32_t endOffset)
  : m_version(version)
  , m_reserve(0)
  , m_gatesOffset(gatesOffset)
  , m_edgesOffset(edgesOffset)
  , m_transfersOffset(transfersOffset)
  , m_linesOffset(linesOffset)
  , m_shapesOffset(shapesOffset)
  , m_networksOffset(networksOffset)
  , m_endOffset(endOffset)
{
}

void TransitHeader::Reset()
{
  m_version = 0;
  m_reserve = 0;
  m_gatesOffset = 0;
  m_edgesOffset = 0;
  m_transfersOffset = 0;
  m_linesOffset = 0;
  m_shapesOffset = 0;
  m_networksOffset = 0;
  m_endOffset = 0;
}

bool TransitHeader::IsEqualForTesting(TransitHeader const & header) const
{
  return m_version == header.m_version
         && m_reserve == header.m_reserve
         && m_gatesOffset == header.m_gatesOffset
         && m_edgesOffset == header.m_edgesOffset
         && m_transfersOffset == header.m_transfersOffset
         && m_linesOffset == header.m_linesOffset
         && m_shapesOffset == header.m_shapesOffset
         && m_networksOffset == header.m_networksOffset
         && m_endOffset == header.m_endOffset;
}

bool TransitHeader::IsValid() const
{
  return m_gatesOffset <= m_edgesOffset && m_edgesOffset <= m_transfersOffset &&
         m_transfersOffset <= m_linesOffset && m_linesOffset <= m_shapesOffset &&
         m_shapesOffset <= m_networksOffset && m_networksOffset <= m_endOffset;
}

// TitleAnchor ------------------------------------------------------------------------------------
TitleAnchor::TitleAnchor(uint8_t minZoom, Anchor anchor) : m_minZoom(minZoom), m_anchor(anchor) {}

bool TitleAnchor::operator==(TitleAnchor const & titleAnchor) const
{
  return m_minZoom == titleAnchor.m_minZoom && m_anchor == titleAnchor.m_anchor;
}

bool TitleAnchor::IsEqualForTesting(TitleAnchor const & titleAnchor) const
{
  return *this == titleAnchor;
}

bool TitleAnchor::IsValid() const
{
  return m_anchor != kInvalidAnchor;
}

// Stop -------------------------------------------------------------------------------------------
Stop::Stop(StopId id, FeatureIdentifiers const & featureIdentifiers, TransferId transferId,
           std::vector<LineId> const & lineIds, m2::PointD const & point,
           std::vector<TitleAnchor> const & titleAnchors)
  : m_id(id)
  , m_featureIdentifiers(featureIdentifiers)
  , m_transferId(transferId)
  , m_lineIds(lineIds)
  , m_point(point)
  , m_titleAnchors(titleAnchors)
{
}

bool Stop::IsEqualForTesting(Stop const & stop) const
{
  double constexpr kPointsEqualEpsilon = 1e-6;
  return m_id == stop.m_id && m_featureIdentifiers.IsEqualForTesting(stop.m_featureIdentifiers) &&
         m_transferId == stop.m_transferId && m_lineIds == stop.m_lineIds &&
         my::AlmostEqualAbs(m_point, stop.m_point, kPointsEqualEpsilon) &&
         m_titleAnchors == stop.m_titleAnchors;
}

bool Stop::IsValid() const
{
  return m_id != kInvalidStopId && !m_lineIds.empty();
}

// SingleMwmSegment -------------------------------------------------------------------------------
SingleMwmSegment::SingleMwmSegment(FeatureId featureId, uint32_t segmentIdx, bool forward)
  : m_featureId(featureId), m_segmentIdx(segmentIdx), m_forward(forward)
{
}

bool SingleMwmSegment::IsEqualForTesting(SingleMwmSegment const & s) const
{
  return m_featureId == s.m_featureId && m_segmentIdx == s.m_segmentIdx && m_forward == s.m_forward;
}

bool SingleMwmSegment::IsValid() const
{
  return m_featureId != kInvalidFeatureId;
}

// Gate -------------------------------------------------------------------------------------------
Gate::Gate(FeatureIdentifiers const & featureIdentifiers, bool entrance, bool exit, double weight,
           std::vector<StopId> const & stopIds, m2::PointD const & point)
  : m_featureIdentifiers(featureIdentifiers)
  , m_entrance(entrance)
  , m_exit(exit)
  , m_weight(weight)
  , m_stopIds(stopIds)
  , m_point(point)
{
}

bool Gate::IsEqualForTesting(Gate const & gate) const
{
  return m_featureIdentifiers.IsEqualForTesting(gate.m_featureIdentifiers) &&
         m_entrance == gate.m_entrance && m_exit == gate.m_exit &&
         my::AlmostEqualAbs(m_weight, gate.m_weight, kWeightEqualEpsilon) &&
         m_stopIds == gate.m_stopIds &&
         my::AlmostEqualAbs(m_point, gate.m_point, kPointsEqualEpsilon);
}

bool Gate::IsValid() const
{
  return m_weight != kInvalidWeight && (m_entrance || m_exit) && !m_stopIds.empty();
}

// ShapeId ----------------------------------------------------------------------------------------
bool ShapeId::operator==(ShapeId const & rhs) const
{
  return m_stop1Id == rhs.m_stop1Id && m_stop2Id == rhs.m_stop2Id;
}

bool ShapeId::IsValid() const
{
  return m_stop1Id != kInvalidStopId && m_stop2Id != kInvalidStopId;
}

// Edge -------------------------------------------------------------------------------------------
Edge::Edge(StopId stop1Id, StopId stop2Id, double weight, LineId lineId, bool transfer,
           std::vector<ShapeId> const & shapeIds)
  : m_stop1Id(stop1Id)
  , m_stop2Id(stop2Id)
  , m_weight(weight)
  , m_lineId(lineId)
  , m_transfer(transfer)
  , m_shapeIds(shapeIds)
{
}

bool Edge::IsEqualForTesting(Edge const & edge) const
{
  return m_stop1Id == edge.m_stop1Id && m_stop2Id == edge.m_stop2Id &&
         my::AlmostEqualAbs(m_weight, edge.m_weight, kWeightEqualEpsilon) &&
         m_lineId == edge.m_lineId && m_transfer == edge.m_transfer &&
         m_shapeIds == edge.m_shapeIds;
}

bool Edge::IsValid() const
{
  if (m_transfer && (m_lineId != kInvalidLineId || !m_shapeIds.empty()))
    return false;

  if (!m_transfer && m_lineId == kInvalidLineId)
    return false;

  return m_stop1Id != kInvalidStopId && m_stop2Id != kInvalidStopId && m_weight != kInvalidWeight &&
         m_lineId != kInvalidLineId;
}

// Transfer ---------------------------------------------------------------------------------------
Transfer::Transfer(StopId id, m2::PointD const & point, std::vector<StopId> const & stopIds,
                   std::vector<TitleAnchor> const & titleAnchors)
  : m_id(id), m_point(point), m_stopIds(stopIds), m_titleAnchors(titleAnchors)
{
}

bool Transfer::IsEqualForTesting(Transfer const & transfer) const
{
  return m_id == transfer.m_id &&
         my::AlmostEqualAbs(m_point, transfer.m_point, kPointsEqualEpsilon) &&
         m_stopIds == transfer.m_stopIds && m_titleAnchors == transfer.m_titleAnchors;
}

bool Transfer::IsValid() const
{
  return m_id != kInvalidStopId && !m_stopIds.empty();
}

// Line -------------------------------------------------------------------------------------------
Line::Line(LineId id, std::string const & number, std::string const & title,
           std::string const & type, NetworkId networkId, std::vector<StopId> const & stopIds)
  : m_id(id)
  , m_number(number)
  , m_title(title)
  , m_type(type)
  , m_networkId(networkId)
  , m_stopIds(stopIds)
{
}

bool Line::IsEqualForTesting(Line const & line) const
{
  return m_id == line.m_id && m_number == line.m_number && m_title == line.m_title &&
         m_type == line.m_type && m_networkId == line.m_networkId && m_stopIds == line.m_stopIds;
}

bool Line::IsValid() const
{
  return m_id != kInvalidLineId && m_networkId != kInvalidNetworkId && !m_stopIds.empty();
}

// Shape ------------------------------------------------------------------------------------------
bool Shape::IsEqualForTesting(Shape const & shape) const
{
  if (!m_id.IsEqualForTesting(shape.m_id) || m_polyline.size() != shape.m_polyline.size())
    return false;

  for (size_t i = 0; i < m_polyline.size(); ++i)
  {
    if (!my::AlmostEqualAbs(m_polyline[i], shape.m_polyline[i], kPointsEqualEpsilon))
      return false;
  }
  return true;
}

// Network ----------------------------------------------------------------------------------------
Network::Network(NetworkId id, std::string const & title)
: m_id(id), m_title(title)
{
}

bool Network::IsEqualForTesting(Network const & shape) const
{
  return m_id == shape.m_id && m_title == shape.m_title;
}

bool Network::IsValid() const
{
  return m_id != kInvalidNetworkId;
}
}  // namespace transit
}  // namespace routing
