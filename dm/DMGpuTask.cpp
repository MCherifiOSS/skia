#include "DMGpuTask.h"

#include "DMComparisonTask.h"
#include "DMUtil.h"
#include "SkCommandLineFlags.h"
#include "SkGpuDevice.h"
#include "SkTLS.h"

namespace DM {

GpuTask::GpuTask(const char* name,
                 Reporter* reporter,
                 TaskRunner* taskRunner,
                 const skiagm::ExpectationsSource& expectations,
                 skiagm::GMRegistry::Factory gmFactory,
                 SkBitmap::Config config,
                 GrContextFactory::GLContextType contextType,
                 int sampleCount)
    : Task(reporter, taskRunner)
    , fGM(gmFactory(NULL))
    , fName(underJoin(fGM->shortName(), name))
    , fExpectations(expectations.get(png(fName).c_str()))
    , fConfig(config)
    , fContextType(contextType)
    , fSampleCount(sampleCount)
    {}

static void* new_gr_context_factory() {
    return SkNEW(GrContextFactory);
}

static void delete_gr_context_factory(void* factory) {
    return SkDELETE((GrContextFactory*) factory);
}

static GrContextFactory* get_gr_factory() {
    return reinterpret_cast<GrContextFactory*>(SkTLS::Get(&new_gr_context_factory,
                                                          &delete_gr_context_factory));
}

void GpuTask::draw() {
    GrContext* gr = get_gr_factory()->get(fContextType);  // Will be owned by device.
    SkGpuDevice device(gr, fConfig, fGM->width(), fGM->height(), fSampleCount);
    SkCanvas canvas(&device);

    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

    SkBitmap bitmap;
    bitmap.setConfig(fConfig, fGM->width(), fGM->height());
    canvas.readPixels(&bitmap, 0, 0);

    // We offload checksum comparison to the main CPU threadpool.
    // This cuts run time by about 30%.
    this->spawnChild(SkNEW_ARGS(ComparisonTask, (*this, fExpectations, bitmap)));
}

bool GpuTask::shouldSkip() const {
    return fGM->getFlags() & skiagm::GM::kSkipGPU_Flag;
}

}  // namespace DM
