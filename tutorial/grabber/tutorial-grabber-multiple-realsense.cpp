/*! \example tutorial-grabber-multiple-realsense.cpp */
#include <visp3/core/vpImage.h>
#include <visp3/gui/vpDisplayGDI.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/gui/vpDisplayX.h>
#include <visp3/sensor/vpRealSense.h>
#include <visp3/sensor/vpRealSense2.h>

/*!
  Grab images from multiple Intel realsense cameras
 */

int main(int argc, char **argv)
{
#if defined(VISP_HAVE_REALSENSE) || defined(VISP_HAVE_REALSENSE2) && (VISP_CXX_STANDARD >= VISP_CXX_STANDARD_11)
    std::vector<std::pair<bool, std::string>> type_serial_nb;
    std::vector<bool> cam_found;

    for (int i = 0; i < argc; i++) 
    {
        if (std::string(argv[i]) == "--T265")
            type_serial_nb.push_back(std::make_pair(true, std::string(argv[i + 1])));
        else if (std::string(argv[i]) == "--D435")
            type_serial_nb.push_back(std::make_pair(false, std::string(argv[i + 1])));
        else if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") 
        {
            std::cout << "\nUsage: " << argv[0]
                << " [--T265 <serial number> ] [--D435 <serial number>]\n"
                << "\nExample to use 2 T265 cameras:\n"
                << "  " << argv[0] << " --T265 11622110511 --T265 11622110433 \n"
                << "\nExample to use 1 T265 and 1 D435 cameras:\n"
                << "  " << argv[0] << " --T265 11622110511 --D435 752112077408 \n"
                << "\nExample to use 2 T265 and 1 D435 cameras:\n"
                << "  " << argv[0] << " --T265 11622110511 --T265 11622110433 --D435 752112070408 \n"
                << std::endl;
            return 0;
        }
    }

    rs2::config T265_cfg, D435_cfg;
    vpRealSense2 g[type_serial_nb.size()];
    vpImage<unsigned char> I[type_serial_nb.size()];

#ifdef VISP_HAVE_X11
    vpDisplayX d[type_serial_nb.size()];
#elif defined(VISP_HAVE_GDI)
    vpDisplayGDI d[type_serial_nb.size()];
#elif defined(VISP_HAVE_OPENCV)
    vpDisplayOpenCV d[type_serial_nb.size()];
#else
    std::cout << "No image viewer is available..." << std::endl;
#endif
    
    bool clicked = false;

    for(int i = 0; i < type_serial_nb.size(); i++)
    {
        if(type_serial_nb[i].first) // T265.
        {
            std::cout << "Opening T265 with ID: " << type_serial_nb[i].second << "." << std::endl;
            T265_cfg.enable_device(type_serial_nb[i].second);
            T265_cfg.enable_stream(RS2_STREAM_FISHEYE, 1, RS2_FORMAT_Y8);
            T265_cfg.enable_stream(RS2_STREAM_FISHEYE, 2, RS2_FORMAT_Y8);
            cam_found.push_back(g[i].open(T265_cfg));
            if(!cam_found.back())
                std::cout << "Device with ID: " << type_serial_nb[i].second << " not found." << std::endl;
        }

        else // D435.
        {
            std::cout << "Opening D435 with ID: " << type_serial_nb[i].second << "." << std::endl;
            D435_cfg.enable_device(type_serial_nb[i].second);
            D435_cfg.disable_stream(RS2_STREAM_DEPTH);
            D435_cfg.disable_stream(RS2_STREAM_INFRARED);
            D435_cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGBA8, 30);
            cam_found.push_back(g[i].open(D435_cfg));
            if(!cam_found.back())
                std::cout << "Device with ID: " << type_serial_nb[i].second << " not found." << std::endl;
        }
    }

    while(true)
    {
        for(int i = 0; i < type_serial_nb.size(); i++)
        {
            if(cam_found[i])
            {
                if(type_serial_nb[i].first) // T265.
                {
                    g[i].acquire(&I[i], NULL, NULL);
                    if(!d[i].isInitialised())
                    {
                        d[i].init(I[i], 900*i, 10, "T265 left image");
                    }
                }

                else // D435.
                {
                    g[i].acquire(I[i]);

                    if(!d[i].isInitialised())
                    {
                        d[i].init(I[i], 900, 10, "D435");
                    }
                }

                vpDisplay::display(I[i]);
                vpDisplay::flush(I[i]);
                clicked = vpDisplay::getClick(I[i], false);
            }

            if(clicked)
                break;
        }

        if(clicked)
            break;
    }
}
#else
  (void) argc;
  (void) argv;
#if !(defined(VISP_HAVE_REALSENSE) || defined(VISP_HAVE_REALSENSE2))
  std::cout << "Install librealsense version > 2.31.0, configure and build ViSP again to use this example" << std::endl;
#endif
#if (VISP_CXX_STANDARD < VISP_CXX_STANDARD_11)
  std::cout << "This turorial should be built with c++11 support" << std::endl;
#endif
#endif