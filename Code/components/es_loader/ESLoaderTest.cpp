#include <gtest/gtest.h>
#include <ESLoader.h>

#include <Records/NPC.h>

// To properly run these tests, move your Skyrim Data\ dir to the binary's dir,
// along with a loadorder.txt in the Data\ dir
namespace
{

class ESLoaderTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ESLoader loader;
        m_collection = loader.BuildRecordCollection();
    }

    UniquePtr<RecordCollection>& GetCollection()
    {
        return m_collection;
    }

    static UniquePtr<RecordCollection> m_collection;
};

UniquePtr<RecordCollection> ESLoaderTest::m_collection = nullptr;

TEST_F(ESLoaderTest, BuildRecordCollection)
{
    auto& pCollection = ESLoaderTest::GetCollection();
    ASSERT_TRUE(pCollection);
}

TEST_F(ESLoaderTest, GetMapMarkerLandmark)
{
    auto& pCollection = ESLoaderTest::GetCollection();

    REFR& mapMarker = pCollection->GetObjectRefById(0x105F3A);
    
    EXPECT_EQ(mapMarker.m_basicObject.m_baseId, 0x10);
    EXPECT_TRUE(mapMarker.m_markerData.m_isMarker);
    EXPECT_EQ(mapMarker.m_markerData.m_marker, Chunks::MapMarkerData::MARKER_TYPE::kLandmark);
}

TEST_F(ESLoaderTest, GetNpcFaendal)
{
    auto& pCollection = ESLoaderTest::GetCollection();

    NPC& faendal = pCollection->GetNpcById(0x13480);
    
    EXPECT_EQ(faendal.m_editorId, "Faendal");
    EXPECT_TRUE(faendal.m_baseStats.IsUnique());
    EXPECT_EQ(faendal.m_defaultOutfit.m_formId, 0xDAB76);
    EXPECT_EQ(faendal.m_scriptData.m_objectFormat, 2);
    EXPECT_EQ(faendal.m_scriptData.m_version, 5);
    EXPECT_EQ(faendal.m_scriptData.m_scriptCount, 2);
}

TEST_F(ESLoaderTest, GetNpcDlcSerana)
{
    auto& pCollection = ESLoaderTest::GetCollection();

    NPC& serana = pCollection->GetNpcById(0x2002B6C);

    EXPECT_EQ(serana.m_editorId, "DLC1Serana");
    EXPECT_EQ(serana.m_defaultOutfit.m_formId, 0x2016903);
}

TEST_F(ESLoaderTest, GetContainerPersonalChest)
{
    auto& pCollection = ESLoaderTest::GetCollection();

    CONT& chest = pCollection->GetContainerById(0x9AF19);

    EXPECT_EQ(chest.m_editorId, "PersonalChestSmall");
    EXPECT_EQ(chest.m_objects.size(), 7);
    EXPECT_EQ(chest.m_objects[0].m_count, 1);
    EXPECT_EQ(chest.m_objects[0].m_formId, 0xAA02C);
}

TEST_F(ESLoaderTest, GetClimateDlcApocrypha)
{
    auto& pCollection = ESLoaderTest::GetCollection();

    CLMT& climate = pCollection->GetClimateById(0x401C48E);

    EXPECT_EQ(climate.m_editorId, "DLC2ApocryphaClimate");
    // TODO: the below check fails. The base id is wrong.
    // Needs to be corrected for the load order, like record form ids.
    EXPECT_EQ(climate.m_weatherList.m_weatherId, 0x4034CFB);
    EXPECT_EQ(climate.m_weatherList.m_chance, 100);
    EXPECT_EQ(climate.m_weatherList.m_globalId, 0);
    EXPECT_EQ(climate.m_sunTexture, "Sky\\Sun.dds");
    EXPECT_EQ(climate.m_glareTexture, "Sky\\SunGlare.dds");
    EXPECT_EQ(climate.m_timing.m_sunriseBegin, 0);
    EXPECT_EQ(climate.m_timing.m_sunriseEnd, 0x9);
    EXPECT_EQ(climate.m_timing.m_sunsetBegin, 0x87);
    EXPECT_EQ(climate.m_timing.m_sunsetEnd, 0x8F);
    EXPECT_EQ(climate.m_timing.m_volatility, 0);
    EXPECT_EQ(climate.m_timing.m_moons, 0x3);
}

} // namespace
