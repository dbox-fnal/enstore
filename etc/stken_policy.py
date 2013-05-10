# This file contains library manager director policies
policydict = {
              'CD-10KCF1.library_manager': {
                                           1: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-FNAL-HISQ',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 8000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 2000, 
                                           'max_waiting_time': 24*3600,
                                           },
                                           2: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-FNAL-HISQ',
                                                    'wrapper': 'cern',
                                                    },
                                           'minimal_file_size': 8000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 2000, 
                                           'max_waiting_time': 2*3600,
                                           },
                                           3: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-hotqcdhisq',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 8000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 2000, 
                                           'max_waiting_time': 24*3600,
                                           },
                                           4: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-nHYPBSM',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 4000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 1000, 
                                           'max_waiting_time': 12*3600,
                                           },
                                           5: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-pndme',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 8000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 8, 
                                           'max_waiting_time': 12*3600,
                                           },
                                           6: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'hpqcd',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 8000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 500, 
                                           'max_waiting_time': 24*3600,
                                           },
                                           7: {'rule': {'storage_group': 'lqcd',
                                                    'file_family': 'lqcd-FNAL-l6496f21b7075m00155m031',
                                                    'wrapper': 'cern',
                                                    },
                                           'minimal_file_size': 50000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50, 
                                           'max_waiting_time': 24*3600,
                                           },
                             },
              'CD-LTO4F1.library_manager': {
                                           1: {'rule': {'storage_group': 'argoneut',
                                                    'file_family': 'rawdata',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 1100,
                                           'max_waiting_time': 24*3600,
                                           },
                                           2: {'rule': {'storage_group': 'argoneut',
                                                    'file_family': 'root_files',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 4000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 100,
                                           'max_waiting_time': 24*3600,
                                           },
                                           3: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'binary-raw-test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 3600,
                                           },
                                           4: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'calibrated-pool-test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 3600,
                                           },
                                           5: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'rawdigits-test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 3600,
                                           },
                                           6: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'reconstructed-pool-test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 3600,
                                           },
                                           7: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'supdigits-test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 200,
                                           'max_waiting_time': 3600,
                                           },
                                           8: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_dst',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 200,
                                           'max_waiting_time': 24*3600,
                                           },
                                           9: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_processing',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 200,
                                           'max_waiting_time': 24*3600,
                                           },
                                           10: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_processing_cal',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 24*3600,
                                           },
                                           11: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_processing_rawdigits',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 24*3600,
                                           },
                                           12: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_processing_sup',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 200,
                                           'max_waiting_time': 24*3600,
                                           },
                                           13: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_reconstructed',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 24*3600,
                                           },
                                           14: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'data_results',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 200,
                                           'max_waiting_time': 24*3600,
                                           },
                                           15: {'rule': {'storage_group': 'minerva',
                                                    'file_family': 'rawdata',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 24*3600,
                                           },
                                           16: {'rule': {'storage_group': 'nova',
                                                    'file_family': 'montecarlo',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50,
                                           'max_waiting_time': 24*3600,
                                           },
                                           17: {'rule': {'storage_group': 'nova',
                                                    'file_family': 'rawdata_FarDet',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50, 
                                           'max_waiting_time': 24*3600,
                                           },
                                           18: {'rule': {'storage_group': 'nova',
                                                    'file_family': 'rawdata_NDOS_unmerged',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50, 
                                           'max_waiting_time': 24*3600,
                                           },
                                           19: {'rule': {'storage_group': 'nova',
                                                    'file_family': 'reco_NDOS',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 5000000000L,
                                           'resulting_library':'CD-DiskSF',
                                           'max_files_in_pack': 50, 
                                           'max_waiting_time': 24*3600,
                                           },
                             },
              'CD-LTO4F1T.library_manager': {
                                           2: {'rule': {'storage_group': 'ssa_test',
                                                    'file_family': 'ssa_test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'max_files_in_pack': 100, 
                                           'max_waiting_time': 120,
                                           'resulting_library':'CD-DiskSFT'
                                           }
                             
                             },
              'CD-LTO4GST.library_manager': {
                                           1: {'rule': {'storage_group': 'ssa_test',
                                                    'file_family': 'ssa_test',
                                                    'wrapper': 'cpio_odc',
                                                    },
                                           'minimal_file_size': 2000000000L,
                                           'max_files_in_pack': 100, 
                                           'max_waiting_time': 120,
                                           'resulting_library':'CD-DiskSFT'
                                           }
                             
                             },
              }
