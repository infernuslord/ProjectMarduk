#include "../soundbank.h"
#include "../soundbank_error.h"
#include "sbtrack.h"
#include "../../asset/world/impl/serialization/cnd/cnd.h"
#include "../../../common.h"
#include "../../../utils/utils.h"

using namespace libim;
using namespace libim::content;
using namespace libim::content::asset;
using namespace libim::content::audio;
using namespace libim::content::audio::impl;
using namespace std::string_view_literals;

struct SoundBank::SoundBankImpl
{
    uint32_t nonceFileId = 0;
    std::vector<SbTrack> vecTracks;

    uint32_t getNextFileId()
    {
        uint32_t v0 = nonceFileId;
        if ( nonceFileId & 1 ){
            v0 = (nonceFileId + 1) % 1111111;
        }
        uint32_t fileId = v0 + 1234;
        nonceFileId = (v0 + 1) % 1111111;
        return fileId;
    }
};


SoundBank::SoundBank(std::size_t nTracks)
{
    ptrImpl_ = std::make_unique<SoundBankImpl>();
   // ptrImpl_->vecTracks.reserve(nTracks);
    ptrImpl_->vecTracks.resize(nTracks);
}

SoundBank::~SoundBank()
{}

std::size_t SoundBank::count() const
{
    return ptrImpl_->vecTracks.size();
}

const std::unordered_map<std::string, Sound>& SoundBank::getTrack(std::size_t trackIdx) const
{
    if(trackIdx >= ptrImpl_->vecTracks.size()) {
        throw SoundBankError("trackIdx out of range!");
    }
    return ptrImpl_->vecTracks.at(trackIdx).sounds;
}

bool SoundBank::importTrack(std::size_t trackIdx, const InputStream& istream)
{
    if(trackIdx >= ptrImpl_->vecTracks.size()) {
        throw SoundBankError("trackIdx out of range!");
    }

    auto& track = ptrImpl_->vecTracks.at(trackIdx);
    if(FileExtMatch(istream.name(), ".cnd"sv))
    {
        auto nonce = CND::ParseSectionSounds(track, istream);
        if(nonce == 0 && !track.sounds.empty()) {
            return false;
        }

        ptrImpl_->nonceFileId = nonce;
        return true;
    }
    else {
        throw SoundBankError("Cannot import soundbank, unknown stream!");
    }
}

