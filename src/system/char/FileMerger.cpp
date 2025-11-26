#include "char/FileMerger.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"

FileMerger *FileMerger::sFmDeleting;

void FileMerger::Merger::Clear(bool b1) {
    mLoaded.Set(FilePath::Root().c_str(), "");
    Hmx::Object *owner = mLoadedObjects.Owner();
    if (owner != sFmDeleting) {
        static Message msg("on_pre_clear", 0);
        msg[0] = mName;
        owner->HandleType(msg);
    }
    while (!mLoadedObjects.empty()) {
        Hmx::Object *front = mLoadedObjects.front();
        delete front;
    }
    ObjectDir *mergerDir = MergerDir();
    if (mergerDir) {
        while (!mLoadedSubdirs.empty()) {
            ObjectDir *curSubdir = mLoadedSubdirs.front();
            mLoadedSubdirs.pop_front();
            mergerDir->RemoveSubDir(curSubdir);
        }
    } else {
        mLoadedSubdirs.clear();
    }
    if (b1 && !TheRnd.GetUnk1b4()) {
        TheRnd.BeginDrawing();
        TheRnd.EndDrawing();
    }
}

FileMerger::FileMerger()
    : mMergers(this), mAsyncLoad(0), mLoadingLoad(0), mCurLoader(0), mFilter(0),
      mHeap(GetCurrentHeapNum()), mOrganizer(this) {
    MILO_ASSERT(MemNumHeaps() == 0 || (mHeap != kNoHeap && mHeap != kSystemHeap), 0x86);
}

FileMerger::~FileMerger() {
    FileMerger *old = sFmDeleting;
    sFmDeleting = this;
    Clear();
    sFmDeleting = old;
}

BEGIN_HANDLERS(FileMerger)
    HANDLE_EXPR(loaded, FindMerger(_msg->Sym(2), true)->mLoaded)
    HANDLE(select, OnSelect)
    HANDLE(start_load, OnStartLoad)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(clear_selections, ClearSelections())
    HANDLE_EXPR(merger_index, FindMergerIndex(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(is_loading, 0)
    HANDLE_ACTION(clear_filter, mFilter = nullptr)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(FileMerger::Merger)
    SYNC_PROP(name, o.mName)
    SYNC_PROP(selected, o.mSelected)
    SYNC_PROP(loaded, o.mLoaded)
    SYNC_PROP(dir, o.mDir)
    SYNC_PROP(proxy, o.mProxy)
    SYNC_PROP(subdirs, (int &)o.mSubdirs)
    SYNC_PROP(preclear, o.mPreClear)
    SYNC_PROP(loaded_objects, o.mLoadedObjects) {
        static Symbol _s("loaded_subdirs");
        if (sym == _s && (_op & (kPropGet | kPropSize)))
            return PropSync(o.mLoadedSubdirs, _val, _prop, _i + 1, _op);
    }
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(FileMerger)
    SYNC_PROP(mergers, mMergers)
    SYNC_PROP(disable_all, sDisableAll)
    SYNC_PROP_SET(loading_load, mLoadingLoad, )
    SYNC_PROP_SET(async_load, mAsyncLoad, )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const FileMerger::Merger &fm) {
    bs << fm.mName;
    bs << fm.mSelected;
    bs << fm.mLoaded;
    bs << fm.mDir;
    bs << fm.mProxy;
    bs << fm.mSubdirs;
    bs << fm.mPreClear;
    return bs;
}

BEGIN_SAVES(FileMerger)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMergers;
END_SAVES

BEGIN_COPYS(FileMerger)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FileMerger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMergers)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FileMerger)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void FileMerger::PreSave(BinStream &) { Clear(); }
void FileMerger::PostSave(BinStream &) { StartLoadInternal(false, false); }

BinStreamRev &operator>>(BinStreamRev &, FileMerger::Merger &);

void FileMerger::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    Hmx::Object::Load(bs);
    if (d.rev < 2) {
        String str;
        d >> str;
    }
    d >> mMergers;
    StartLoadInternal(true, true);
}

void FileMerger::FinishLoading(Loader *ldr) {
    DirLoader *dl = dynamic_cast<DirLoader *>(ldr);
    Merger *merger = NotifyFileLoaded(ldr, dl);
    if (dl && !sDisableAll) {
        if (merger->mProxy) {
            MILO_ASSERT(dl->GetDir(), 0x236);
            ObjectDir *dir = Dir()->Find<ObjectDir>(dl->GetDir()->Name(), false);
            if (dir) {
                ReserveToFit(dl->GetDir(), dir, 0);
                MergeDirs(dl->GetDir(), dir, *this);
                dir->SyncObjects();
            } else {
                ObjectDir *dlDir = dl->GetDir();
                ReserveToFit(nullptr, Dir(), 2);
                dlDir->SetName(dlDir->Name(), Dir());
                merger->mLoadedObjects.push_back(dlDir);
            }
        } else {
            ObjectDir *mergerDir = merger->MergerDir();
            ReserveToFit(dl->GetDir(), mergerDir, 0);
            MergeDirs(dl->GetDir(), mergerDir, *this);
        }
    }
    PostMerge(merger, dl, true);
}

void FileMerger::FailedLoading(Loader *l) {
    MILO_ASSERT(l == mCurLoader, 0x204);
    MILO_ASSERT(l->LoaderFile() == mFilesPending.front()->loading, 0x205);
    static Message msg("on_load_failed", 0);
    msg[0] = mFilesPending.front()->mName;
    HandleType(msg);
    PostMerge(mFilesPending.front(), dynamic_cast<DirLoader *>(l), false);
}

MergeFilter::Action FileMerger::Filter(Hmx::Object *o1, Hmx::Object *o2, ObjectDir *dir) {
    Action a;
    if (mFilter) {
        a = mFilter->Filter(o1, o2, dir);
    } else {
        a = MergeAction(o1, o2, dir);
    }
    if (a == 1 && !o2) {
        mFilesPending.front()->mLoadedObjects.push_back(o1);
    }
    return a;
}

MergeFilter::SubdirAction FileMerger::FilterSubdir(ObjectDir *o1, ObjectDir *o2) {
    SubdirAction a;
    if (mFilter) {
        a = mFilter->FilterSubdir(o1, o2);
    } else {
        a = MergeFilter::DefaultSubdirAction(o1, mFilesPending.front()->mSubdirs);
    }
    if (a == 1 && !o2->HasSubDir(o1)) {
        mFilesPending.front()->mLoadedSubdirs.push_back(o1);
    }
    return a;
}

bool FileMerger::OriginalPath(Hmx::Object *obj, String &str) {
    Merger *merger = InMerger(obj);
    if (merger) {
        str = merger->mLoaded;
        return true;
    } else {
        return false;
    }
}

void FileMerger::Clear() {
    for (int i = 0; i < mMergers.size(); i++) {
        mMergers[i].Clear(false);
    }
    if (mCurLoader) {
        Merger *merger = mFilesPending.front();
        mFilesPending.clear();
        mFilesPending.push_front(merger);
        DeleteCurLoader();
    }
}

bool FileMerger::StartLoad(bool b) { return StartLoadInternal(b, false); }

FileMerger::Merger *FileMerger::FindMerger(Symbol s, bool b) {
    int idx = FindMergerIndex(s, b);
    if (idx != -1) {
        return &mMergers[idx];
    } else {
        return nullptr;
    }
}

void FileMerger::ClearSelections() {
    for (int i = 0; i < mMergers.size(); i++) {
        mMergers[i].mSelected.Set(FilePath::Root().c_str(), "");
    }
    if (mCurLoader) {
        Merger *front = mFilesPending.front();
        mFilesPending.clear();
        mFilesPending.push_front(front);
        DeleteCurLoader();
    }
}

int FileMerger::FindMergerIndex(Symbol name, bool warn) {
    for (int i = 0; i < mMergers.size(); i++) {
        if (mMergers[i].mName == name) {
            return i;
        }
    }
    if (warn) {
        MILO_NOTIFY("%s could not find Merger %s", PathName(this), name);
    }
    return -1;
}

FileMerger::Merger *FileMerger::InMerger(Hmx::Object *o) {
    for (int i = 0; i < mMergers.size(); i++) {
        Merger &cur = mMergers[i];
        if (cur.IsObjectLoaded(o)) {
            return &cur;
        }
    }
    return nullptr;
}

void FileMerger::DeleteCurLoader() {
    if (mCurLoader) {
        DirLoader *d = dynamic_cast<DirLoader *>(mCurLoader);
        // if (d)
        //     d->unk99 = true;
        delete mCurLoader;
    }
}
