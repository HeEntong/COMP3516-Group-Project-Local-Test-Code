# plt.plot(acf_plot[0], label='ACF G')
# plt.plot(acf_plot[1], label='ACF 0')

# plt.savefig('acf_plot.pdf')



# subcarrier_csi_sample = subacarrier_csi[10]  # 5th subcarrier
# ACF_data = []
# for tau in range(50):
#     acf_tau = sample_auto_covariance(subcarrier_csi_sample, tau)
#     ACF_data.append(acf_tau)
# plt.clf()
# plt.plot(ACF_data)
# plt.savefig('acf_plot.png')