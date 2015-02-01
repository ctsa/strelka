// -*- mode: c++; indent-tabs-mode: nil; -*-
//
// Starka
// Copyright (c) 2009-2014 Illumina, Inc.
//
// This software is provided under the terms and conditions of the
// Illumina Open Source Software License 1.
//
// You should have received a copy of the Illumina Open Source
// Software License 1 along with this program. If not, see
// <https://github.com/sequencing/licenses/>
//

/// \file

/// \author Chris Saunders
///

#include "htsapi/bam_dumper.hh"
#include "starling_common/starling_streams_base.hh"

#include <cassert>

#include <fstream>
#include <iostream>



bam_dumper*
starling_streams_base::
initialize_realign_bam(const bool is_clobber,
                       const prog_info& pinfo,
                       const std::string& filename,
                       const char* label,
                       const bam_header_t* const header)
{
    assert(nullptr != header);

    // \TODO consider putting extra info into BAM header:
    //
    //fp->header = bam_header_dup((const bam_header_t*)aux);
    //fos << "@PG\tID:" << pinfo.name() << "\tVN:" << pinfo.version() << "\tCL:" << cmdline << "\n";

    if (! is_clobber)   // weak clobber test:
    {
        std::ofstream fos;
        open_ofstream(pinfo,filename,label,is_clobber,fos);
    }
    return new bam_dumper(filename.c_str(),header);
}



std::ostream*
starling_streams_base::
initialize_candidate_indel_file(const starling_base_options& opt,
                                const prog_info& pinfo,
                                const std::string& filename)
{
    const char* const cmdline(opt.cmdline.c_str());

    std::ofstream* fosptr(new std::ofstream);
    std::ofstream& fos(*fosptr);
    open_ofstream(pinfo,filename,"candidate-indel",opt.is_clobber,fos);

    fos << "# ** " << pinfo.name();
    fos << " candidate-indel file **\n";
    write_file_audit(opt,pinfo,cmdline,fos);
    fos << "#\n";
    fos << "#$ COLUMNS seq_name pos type length seq\n";

    return fosptr;
}



std::ostream*
starling_streams_base::
initialize_window_file(const starling_base_options& opt,
                       const prog_info& pinfo,
                       const avg_window_data& awd,
                       const sample_info& si)
{
    const char* const cmdline(opt.cmdline.c_str());

    std::ofstream* fosptr(new std::ofstream);
    std::ofstream& fos(*fosptr);
    open_ofstream(pinfo,awd.filename,"variant-window",opt.is_clobber,fos);

    const unsigned fs(awd.flank_size);

    fos << "# ** " << pinfo.name();
    fos << " variant-window file **\n";
    write_file_audit(opt,pinfo,cmdline,fos);
    fos << "#$ FLANK_SIZE " << awd.flank_size << "\n";
    fos << "#\n";
    fos << "#$ COLUMNS seq_name pos";
    static const bool is_tier1(true);
    static const char* win_type[] = {"used","filt","submap"};
    static unsigned n_win_type(sizeof(win_type)/sizeof(char*));
    const unsigned n_samples(si.sample_size());
    for (unsigned s(0); s<n_samples; ++s)
    {
        for (unsigned i(0); i<n_win_type; ++i)
        {
            fos << " " << si.get_prefix(s,is_tier1) << "win" << fs << "_" << win_type[i];
        }
    }
    fos << "\n";

    return fosptr;
}



starling_streams_base::
starling_streams_base(const starling_base_options& opt,
                      const prog_info& pinfo,
                      const sample_info& si)
    : base_t(opt,pinfo,true)
    , _n_samples(si.sample_size())
    , _window_osptr(opt.variant_windows.size())
{
    assert((_n_samples>0) && (_n_samples<=MAX_SAMPLE));

    if (opt.is_write_candidate_indels())
    {
        _candidate_indel_osptr.reset(initialize_candidate_indel_file(opt,pinfo,opt.candidate_indel_filename));
    }

    const unsigned vs(opt.variant_windows.size());
    for (unsigned i(0); i<vs; ++i)
    {
        _window_osptr[i].reset(initialize_window_file(opt,pinfo,opt.variant_windows[i],si));
    }
}
