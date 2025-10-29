#include "obj/DataFile.h"
#include "math/FileChecksum.h"
#include "obj/Data.h"
#include "obj/DataFile_Flex.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/ThreadCall.h"
#include "utl/BufStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

CriticalSection gDataReadCrit; // yes these are the bss offsets. this tu sucks
// DataArray *gArray; // 0x28
// int gNode; // 0x2c
Symbol gFile; // 0x30
// BinStream *gBinStream; // 0x34
// int gOpenArray = kDataTokenFinished; // 0x38 ?
// std::list<ConditionalInfo> gConditional; // 0x48 - actually a list of ConditionalInfo
//                                          // structs
int gDataLine; // 0x50
// std::map<String, DataNode> gReadFiles; // 0x60

// bool gCompressCached;
// bool gCachingFile;
// bool gReadingFile;

DataArray *ReadCacheStream(BinStream &bs, const char *cc) {
    CritSecTracker cst(&gDataReadCrit);
    bs.EnableReadEncryption();
    DataArray::SetFile(cc);
    DataArray *arr;
    bs >> arr;
    bs.DisableEncryption();
    return arr;
}

void DataFail(const char *msg) {
    MILO_FAIL("%s (file %s, line %d)", msg, gFile, gDataLine);
}

DataLoader::DataLoader(const FilePath &fp, LoaderPos pos, bool b3)
    : Loader(fp, pos), mFilename(""), mData(nullptr), mFile(nullptr), mBufLen(0),
      mBuffer(nullptr), mDtb(b3), mThreadObj(nullptr) {
    const char *new_str = fp.c_str();
    if (b3) {
        new_str = CachedDataFile(new_str, mDtb);
    }
    mFilename = new_str;
    mState = &DataLoader::OpenFile;
}

DataLoader::~DataLoader() {
    if (mState != &DataLoader::DoneLoading) {
        delete mFile;
        MemFree(mBuffer);
    } else if (mData) {
        mData->Release();
    }
}

bool DataLoader::IsLoaded() const { return mState == &DataLoader::DoneLoading; }
void DataLoader::PollLoading() { (this->*mState)(); }
void DataLoader::DoneLoading() {}

void DataLoader::OpenFile() {
    mFile = NewFile(mFilename.c_str(), 2);
    if (mFile && !mFile->Fail()) {
        mBufLen = mFile->Size();
        mBuffer = (char *)MemAlloc(mBufLen, __FILE__, 0x366, "Resource");
        mFile->ReadAsync(mBuffer, mBufLen);
        mState = &DataLoader::LoadFile;
    } else {
        if (!mFilename.empty()) {
            MILO_NOTIFY("Could not load: %s", FileLocalize(LoaderFile().c_str(), nullptr));
        }
        mState = &DataLoader::DoneLoading;
    }
}

DataArray *DataLoader::Data() {
    MILO_ASSERT(IsLoaded(), 0x3A5);
    return mData;
}

void DataLoader::ThreadDone(DataArray *arr) {
    MILO_ASSERT(MainThread(), 0x3B5);
    mData = arr;
    RELEASE(mThreadObj);
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x3BA);
        mBuffer = nullptr;
    }
    RELEASE(mFile);
    mState = &DataLoader::DoneLoading;
}

void DataLoader::LoadFile() {
    if (mThreadObj) {
        Timer::Sleep(0);
    } else {
        int x;
        if (mFile->ReadDone(x)) {
            if (mFile->Fail()) {
                ThreadDone(nullptr);
            } else {
                mThreadObj = new DataLoaderThreadObj(
                    this, mFile, (char *)mBuffer, mBufLen, mDtb, mFilename.c_str()
                );
                ThreadCall(mThreadObj);
            }
        }
    }
}

DataLoaderThreadObj::DataLoaderThreadObj(
    DataLoader *dl, File *file, char *buffer, int bufSize, bool dtb, const char *filename
)
    : mLoader(dl), unk8(nullptr), mFile(file), mBufLen(bufSize), mBuffer(buffer),
      mFilename(filename), mDtb(dtb), mLocal(FileIsLocal(filename)) {}

int DataLoaderThreadObj::ThreadStart() {
    BufStream bs(mBuffer, mBufLen, true);
    bs.SetName(FileLocalize(mLoader->LoaderFile().c_str(), nullptr));
    if (mDtb) {
        bool runChecksum = HasFileChecksumData() && !mLocal;
        if (runChecksum) {
            bs.StartChecksum(mFilename);
        }
        unk8 = ReadCacheStream(bs, mLoader->LoaderFile().c_str());
        if (runChecksum) {
            bs.ValidateChecksum();
        }
    } else {
        unk8 = DataReadStream(&bs);
    }
    return 0;
}

void DataLoaderThreadObj::ThreadDone(int) { mLoader->ThreadDone(unk8); }
